#include "stats.h"
#include "jsonutil.h"
#include <stack>
#include <string>
#include <vector>
#include <functional>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Container/Str.h>

static StringHasherType shasher;

static FunctionMapType modfuncs=
{
	{
		shasher("test"),
		{
			2,
			[](const double *a, unsigned int n)->double{Log::Write(LOG_INFO, "test"); return (n>=2) ? (a[0]+a[1]*400) : 0;}
		}
	},

	{
		shasher("logistic"),
		{
			3,
			[](const double *a, unsigned int n)->double
			{
				double e=2.71828;
				if(n!=3) return 0.0;
				double stat=a[0];
				double base=a[1];
				double slope=a[2];

				return (1.0 / (1.0 + std::pow(e, -slope * (stat-base))));
			}
		}
	},

	{
		shasher("spread"),
		{
			3,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				double base=a[1];
				double spread=a[2];

				double low=base-spread;
				double high=base+spread;
				double v=stat;
				v = (v-low) / (high - low);
				return v;
			}
		}
	},

	{
		shasher("center"),
		{
			1,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				return stat*2.0-1.0;
			}
		}
	},

	{
		shasher("clamp"),
		{
			3,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				double low=a[1];
				double high=a[2];

				return std::max(low, std::min(high, stat));
			}
		}
	},

	{
		shasher("min"),
		{
			2,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				double val=a[1];

				return std::min(stat,val);
			}
		}
	},

	{
		shasher("max"),
		{
			2,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				double val=a[1];

				return std::max(stat,val);
			}
		}
	},

	{
		shasher("pow"),
		{
			2,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];
				double val=a[1];

				return std::pow(stat,val);
			}
		}
	},

	{
		shasher("saturate"),
		{
			1,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];

				return std::max(0.0, std::min(1.0, stat));
			}
		}
	},

	{
		shasher("saturatecenter"),
		{
			1,
			[](const double *a, unsigned int n)->double
			{
				double stat=a[0];

				stat=stat*2.0-1.0;
				return std::max(-1.0, std::min(1.0, stat));
			}
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

	/*auto p=pf.ToPostfix();
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
		Log::Write(LOG_ERROR, "JSON file not an object");
		return; // Needs to be an object
	}

	const JSONObject &obj=json.GetObject();
	for(auto i=obj.Begin(); i!=obj.End(); ++i)
	{
		// name of stat being built is i->First
		// array of mods is i->Second
		std::string name=i->first_.CString();
		const JSONValue &val=i->second_;

		if(val.IsArray())
		{
			const JSONArray &mods=val.GetArray();
			for(unsigned int m=0; m<mods.Size(); ++m)
			{
				const JSONValue &md=mods[m];
				if(md.IsArray())
				{
					const JSONArray &mod=md.GetArray();
					if(mod.Size()>=2)
					{
						const String type=mod[0].GetString();//GetStringFromJSONObject("Type", mod);
						const std::string expr=mod[1].GetString().CString();//GetStringFromJSONObject("Exp", mod).CString();
						StatModifier::Type tp=StatModifier::FLAT;
						if(type=="Mult") tp=StatModifier::MULT;
						else if(type=="Scale") tp=StatModifier::SCALE;
						else if(type=="Min") tp=StatModifier::MIN;
						else if(type=="Max") tp=StatModifier::MAX;

						AddMod(name, tp, expr);
					}
					else
					{
						Log::Write(LOG_ERROR, String("Mod definition array is too short in list for ") + i->first_);
					}
				}
				else
				{
					Log::Write(LOG_ERROR, String("Mod definition is not an array in list for ") + i->first_);
				}
			}
		}
		else
		{
			Log::Write(LOG_ERROR, String("Mod list for ") + i->first_ + " is not an array");
		}
	}
}

void StatSet::Merge(const StatSet &rhs)
{
	for(auto i : rhs.stats_)
	{
		for(auto j : i.second)
		{
			AddMod(i.first, j);
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
		i->ConcatenateStat(s, stat);
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
	static const StringHashType carat(shasher("^")), multiply(shasher("*")), divide(shasher("/")), plus(shasher("+")), minus(shasher("-"));
	std::stack<double> stk;

	String st;

	for(auto i : mod.expr_)
    {
        if(i.GetType()==Token::NUMBER)
        {
            //stk.push(std::stod(i.GetToken()));
			stk.push(i.GetValue());
			//Log::Write(LOG_INFO, String("Pushed number: ") + String(i.GetValue()));
        }
        else if(i.GetType()==Token::OPERATOR)
        {
            double right=stk.top();
            stk.pop();
            double left=stk.top();
            stk.pop();

			//Log::Write(LOG_INFO, String("Popped numbers :") + String(left) + "," + String(right));
            if(i.GetToken()==plus)
			{
				stk.push(left+right);
				//Log::Write(LOG_INFO, String("Pushed add: ") + String(left+right));
			}
            else if(i.GetToken()==minus)
			{
				stk.push(left-right);
				//Log::Write(LOG_INFO, String("Pushed subtract: ") + String(left-right));
			}
            else if(i.GetToken()==multiply)
			{
				stk.push(left*right);
				//Log::Write(LOG_INFO, String("Pushed mult: ") + String(left*right));
			}
            else if(i.GetToken()==divide)
			{
				stk.push(left/right);
				//Log::Write(LOG_INFO, String("Pushed div: ") + String(left/right));
			}
            else if(i.GetToken()==carat)
			{
				stk.push(std::pow(left,right));
				//Log::Write(LOG_INFO, String("Pushed exp: ") + String(std::pow(left,right)));
			}
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
				int numargs=fn.numargs_;
				if(numargs>0)
				{
					//double ar[numargs];
					std::vector<double> ar(numargs);
					for(unsigned int m=0; m<numargs; ++m)
					{
						ar[(numargs-1)-m]=stk.top();
						stk.pop();
					}
					stk.push(fn.func_(&ar[0],numargs));
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
			double v=GetStatValue(stats,i.GetToken());
           stk.push(v);
		   //Log::Write(LOG_INFO, String("Pushed var token value: ") + String(v));
        }
    }

    return stk.top();
}

StatSetCollectionSnapshot::StatSetCollectionSnapshot(const StatSetCollection &collection) : statsets_(collection.size()), collection_(collection)
{
	unsigned int i=0;
	for(auto ss : collection)
	{
		statsets_[i++]=*ss;
	}
}

DamageBoostValues GetDamageBoosts(const StatSetCollection &stats, std::string stat)
{
	static StringHasherType hasher;
	StringHashType h=hasher(stat);
	return GetDamageBoosts(stats,h);
}

DamageBoostValues GetDamageBoosts(const StatSetCollection &stats, StringHashType stat)
{
	Stat s;

	for(auto &i : stats)
	{
		i->ConcatenateStat(s, stat);
	}

	double mult=0.0, scale=1.0;

	for(auto &j : s)
	{
		double val=EvaluateStatMod(stats, j);
		switch(j.type_)
		{
			case StatModifier::MULT: mult+=val; break;
			case StatModifier::SCALE: scale*=(1.0+val); break;
			default: break;
		}
	}

	return DamageBoostValues(mult, scale);
}
