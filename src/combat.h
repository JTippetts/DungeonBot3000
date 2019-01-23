#pragma once

// Combat resolution
#include "stats.h"
#include <vector>
#include <string>

enum DamageTypes
{
	DPhysical,
	DBurn,
	DElectrical,
	DNumTypes
};

struct DamageRange
{
	int type_;
	double low_, high_;

	DamageRange(int type, double low, double high) : type_(type), low_(low), high_(high){}
};
using DamageRangeList=std::vector<DamageRange>;

struct DamageValue
{
	int type_;
	double value_;

	DamageValue(int type, double val) : type_(type), value_(val){}
};
using DamageValueList=std::vector<DamageValue>;

extern std::vector<std::string> DamageNames;

bool MakeHitRoll(StatSetCollection &attacker, StatSetCollection &defender);
DamageValueList BuildDamageList(StatSetCollection &attacker);
DamageValueList BuildDamageList(StatSetCollection &attacker, DamageRangeList &damage);
DamageValueList BuildDamageList(StatSetCollection &attacker, DamageValue &damage);

DamageValueList ProcessIncomingDamage(StatSetCollection &defender, const DamageValueList &damages);

double ProcessHoT(StatSetCollection &attacker, double hot);
double ProcessIncomingHoT(StatSetCollection &defender, double hot);
