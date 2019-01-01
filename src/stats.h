#pragma once

#include "parsing.h"
#include <list>
#include <unordered_map>

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/JSONValue.h>
// Combat Stat Modifier

struct StatModifier
{
	enum Type
	{
		FLAT,
		MULT,
		SCALE,
		MIN,
		MAX
	};

	Type type_;
	TokenStream expr_;

	StatModifier() : type_(FLAT){}
	StatModifier(Type type, const TokenStream &expr) : type_(type), expr_(expr){}
	StatModifier(const StatModifier &rhs) : type_(rhs.type_), expr_(rhs.expr_){}

	void Set(Type type, const TokenStream &expr)
	{
		type_=type;
		expr_=expr;
	}
};

using Stat = std::list<StatModifier>;

struct StatModifierHandle
{
	Stat *list_;
	Stat::iterator iter_;

	StatModifierHandle(): list_(nullptr){}
	StatModifierHandle(Stat *l, Stat::iterator i) : list_(l), iter_(i){}
	StatModifierHandle(const StatModifierHandle &rhs) : list_(rhs.list_), iter_(rhs.iter_){}
};
//using StatModifierHandle = std::list<StatModifier>::iterator;

class StatSet
{
	public:
	StatSet();
	StatSet(const StatSet &rhs);

	StatModifierHandle AddMod(std::string stat, const StatModifier &mod);
	StatModifierHandle AddMod(StringHashType stat, const StatModifier &mod);
	StatModifierHandle AddMod(std::string stat, StatModifier::Type type, const std::string &expr);
	StatModifierHandle AddMod(StringHashType stat, StatModifier::Type type, const std::string &expr);
	void RemoveMod(std::string stat, StatModifierHandle mod);
	void RemoveMod(StringHashType stat, StatModifierHandle mod);

	void LoadJSON(const Urho3D::JSONValue &json);

	void ConcatenateStat(Stat &stat, std::string name) const;
	void ConcatenateStat(Stat &stat, StringHashType name) const;

	protected:
	std::unordered_map<StringHashType, Stat> stats_;
};

using StatSetCollection = std::vector<StatSet>;

double GetStatValue(const StatSetCollection &stats, std::string stat);
double GetStatValue(const StatSetCollection &stats, StringHashType stat);
double EvaluateStatMod(const StatSetCollection &stats, const StatModifier &mod);

