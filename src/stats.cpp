#include "stats.h"
#include "jsonutil.h"
#include <stack>
#include <string>
#include <functional>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Container/Str.h>

static StringHasherType shasher;

static FunctionMapType modfuncs=
{
	{
		"test",
		{
			2,
			[](const double *a, unsigned int n)->double{Log::Write(LOG_INFO, "test"); return (n>=2) ? (a[0]+a[1]*400) : 0;}
		}
	}
};

StatSet::StatSet()
{
}

StatSet::StatSet(const StatSet &rhs) : stats_(rhs.stats_){}

StatModifierHandle StatSet::AddMod(std::string stat, const StatModifier &mod)
{
	static StringHasherType hasher;
	StringHashType h=hasher(stat);
	return AddMod(h,mod);
}

StatModifierHandle StatSet::AddMod(StringHashType stat, const StatModifier &mod)
{
	Stat &s=stats_[stat];
	s.push_back(mod);
	return StatModifierHandle(&s,--s.end());
}

StatModifierHandle StatSet::AddMod(std::string stat, StatModifier::Type type, const std::string &expr)
{
	static StringHasherType hasher;
	StringHashType h=hasher(stat);
	return AddMod(h,type,expr);
}

StatModifierHandle StatSet::AddMod(StringHashType stat, StatModifier::Type type, const std::string &expr)
{
	Stat &s=stats_[stat];

	ExpressionToPostfix pf(expr, modfuncs);
	s.push_back(StatModifier(type, pf.ToPostfix()));

	// Debug
	/*
	auto p=pf.ToPostfix();
	for(auto i : p)
	{
		Log::Write(LOG_INFO, String(i.GetToken().c_str()) + String(" ") + String((int)i.GetType()));
	}*/

	return StatModifierHandle(&s,--s.end());
}

void StatSet::RemoveMod(std::string stat, StatModifierHandle mod)
{
	static StringHasherType hasher;
	StringHashType h=hasher(stat);
	RemoveMod(h,mod);
}

void StatSet::RemoveMod(StringHashType stat, StatModifierHandle mod)
{
	Stat &s=stats_[stat];
	if(mod.list_ != &s) return;
	s.erase(mod.iter_);
}

void StatSet::LoadJSON(const Urho3D::JSONValue &json)
{
	StringHasherType StringHasher;
	if(!json.IsObject())
	{
		return; // Needs to be an object
	}

	const JSONObject &obj=json.GetObject();
	for(auto i=obj.Begin(); i!=obj.End(); ++i)
	{
		// name of stat being built is i->First
		// array of mods is i->Second
		std::string name=i->first_.CString();
		const JSONValue &val=i->second_;
		//BaseStat &stat=GetStat(name);
		if(val.IsArray())
		{
			const JSONArray &mods=val.GetArray();
			for(unsigned int m=0; m<mods.Size(); ++m)
			{
				const JSONValue &md=mods[m];
				if(md.IsObject())
				{
					const JSONObject &mod=md.GetObject();

					const String type=GetStringFromJSONObject("Type", mod);
					const std::string expr=GetStringFromJSONObject("Exp", mod).CString();

					StatModifier::Type tp=StatModifier::FLAT;
					if(type=="Mult") tp=StatModifier::MULT;
					else if(type=="Scale") tp=StatModifier::SCALE;
					else if(type=="Min") tp=StatModifier::MIN;
					else tp=StatModifier::MAX;

					AddMod(name, tp, expr);
				}
			}
		}
	}
}

void StatSet::ConcatenateStat(Stat &stat, std::string name) const
{
	static StringHasherType hasher;
	StringHashType h=hasher(name);
	ConcatenateStat(stat,h);
}

void StatSet::ConcatenateStat(Stat &stat, StringHashType name) const
{
	auto f=stats_.find(name);
	if(f==stats_.end()) return;

	const Stat &s=(*f).second;

	for(auto &i : s) stat.push_back(i);
}

double GetStatValue(const StatSetCollection &stats, std::string stat)
{
	static StringHasherType hasher;
	StringHashType h=hasher(stat);
	return GetStatValue(stats,h);
}

double GetStatValue(const StatSetCollection &stats, StringHashType stat)
{
	Stat s;

	for(auto &i : stats)
	{
		i.ConcatenateStat(s, stat);
	}

	double flat=0.0, mult=0.0, scale=1.0, mn=0, mx=0;
	bool hasmin=false;
	bool hasmax=false;
	for(auto &j : s)
	{
		double val=EvaluateStatMod(stats, j);
		switch(j.type_)
		{
			case StatModifier::FLAT: flat+=val; break;
			case StatModifier::MULT: mult+=val; break;
			case StatModifier::SCALE: scale*=(1.0+val); break;
			case StatModifier::MIN: mn=val; hasmin=true; break;
			case StatModifier::MAX: mx=val; hasmax=true; break;
		}
	}

	double ret=flat*(1.0+mult)*scale;
	if(hasmin) ret=std::max(mn,ret);
	if(hasmax) ret=std::min(mx,ret);
	return ret;
}

double EvaluateStatMod(const StatSetCollection &stats, const StatModifier &mod)
{
	std::stack<double> stk;

	for(auto i : mod.expr_)
    {
        if(i.GetType()==Token::NUMBER)
        {
            stk.push(std::stod(i.GetToken()));
        }
        else if(i.GetType()==Token::OPERATOR)
        {
            double right=stk.top();
            stk.pop();
            double left=stk.top();
            stk.pop();
            if(i.GetToken()=="+") stk.push(left+right);
            else if(i.GetToken()=="-") stk.push(left-right);
            else if(i.GetToken()=="*") stk.push(left*right);
            else if(i.GetToken()=="/") stk.push(left/right);
            else if(i.GetToken()=="^") stk.push(std::pow(left,right));
        }
        else if(i.GetType()==Token::UNARYOPERATOR)
        {
            double o=stk.top();
            stk.pop();
            stk.push(o*-1.0);
        }
        else if(i.GetType()==Token::FUNCTION)
        {
			auto fi=modfuncs.find(i.GetToken());
			if(fi != modfuncs.end())
			{
				auto &fn = (*fi).second;
				const int numargs=fn.numargs_;
				if(numargs>0)
				{
					double ar[numargs];
					for(unsigned int m=0; m<numargs; ++m)
					{
						ar[(numargs-1)-m]=stk.top();
						stk.pop();
					}
					stk.push(fn.func_(ar,numargs));
				}
				else
				{
					stk.push(fn.func_(nullptr,0));
				}
			}
			else
			{
				// Function not found
			}
        }
        else if(i.GetType()==Token::VAR)
        {
           stk.push(GetStatValue(stats, shasher(i.GetToken())));
        }
    }

    return stk.top();
}
