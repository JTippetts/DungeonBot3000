#include "combat.h"

double rolld(double low, double high);


void BoostDamage(StatSetCollection &attacker, DamageValue &damage)
{
	// Collect mults and scales
	double increased=1.0+GetStatValue(attacker, "IncreasedDamage");
	increased += GetStatValue(attacker, "Increased" + DamageNames[damage.type_] + "Damage");
	if(damage.type_==DBurn || damage.type_==DElectrical) increased += GetStatValue(attacker, "IncreasedElementalDamage");

	double more=1.0+GetStatValue(attacker, "MoreDamage");
	more *= 1.0 + GetStatValue(attacker, "More" + DamageNames[damage.type_] + "Damage");
	if(damage.type_==DBurn || damage.type_==DElectrical) more *= 1.0 + GetStatValue(attacker, "MoreElementalDamage");

	damage.value_ = std::max(0.0,damage.value_ * increased * more);
}

void BoostIncomingDamage(StatSetCollection &defender, DamageValue &damage)
{
	double increased=1.0+GetStatValue(defender, "IncreasedDamageTaken");
	increased += GetStatValue(defender, "Increased" + DamageNames[damage.type_] + "DamageTaken");
	if(damage.type_==DBurn || damage.type_==DElectrical) increased += GetStatValue(defender, "IncreasedElementalDamageTaken");

	increased -= GetStatValue(defender, "DecreasedDamageTaken");
	increased -= GetStatValue(defender, "Decreased" + DamageNames[damage.type_] + "DamageTaken");
	if(damage.type_==DBurn || damage.type_==DElectrical) increased -= GetStatValue(defender, "DecreasedElementalDamageTaken");

	double more=1.0+GetStatValue(defender, "MoreDamageTaken");
	more *= 1.0 + GetStatValue(defender, "More" + DamageNames[damage.type_] + "DamageTaken");
	if(damage.type_==DBurn || damage.type_==DElectrical) more *= 1.0 + GetStatValue(defender, "MoreElementalDamageTaken");

	more *= 1.0 - GetStatValue(defender, "LessDamageTaken");
	more *= 1.0 - GetStatValue(defender, "Less" + DamageNames[damage.type_] + "DamageTaken");
	if(damage.type_==DBurn || damage.type_==DElectrical) more *= 1.0 - GetStatValue(defender, "LessElementalDamageTaken");

	damage.value_ = std::max(0.0,damage.value_ * increased * more);
}

void ConvertDamage(StatSetCollection &attacker, DamageValueList &out, DamageValue &damage, int level)
{
	BoostDamage(attacker, damage);
	if(level<0) return;
	double conversion=1.0;
	DamageValue td=damage;

	// Damage conversions
	for(int d=DPhysical; d<DNumTypes; ++d)
	{
		if(d!=damage.type_)
		{
			double convamount=GetStatValue(attacker, DamageNames[damage.type_] + "As" + DamageNames[d]);
			if(convamount>0.0)
			{
				convamount=std::min(conversion,convamount); // Don't allow conversion to overflow for extra damage
				conversion-=convamount;
				DamageValue newd(d, td.value_ * convamount);
				td.value_ -= td.value_ * convamount;
				ConvertDamage(attacker, out, newd, level-1);
			}
		}
	}

	//Extra damages
	for(int d=DPhysical; d<DNumTypes; ++d)
	{
		if(d!=damage.type_)
		{
			double a=GetStatValue(attacker, DamageNames[damage.type_] + "AsExtra" + DamageNames[d]);
			if(a>0.0)
			{
				DamageValue newd(d, td.value_ * a);
				ConvertDamage(attacker, out, newd, level-1);
			}
		}
	}

	out.push_back(td);
}

void ConvertIncomingDamage(StatSetCollection &defender, DamageValueList &out, DamageValue &damage, int level)
{
	BoostIncomingDamage(defender, damage);
	if(level<0) return;
	double conversion=1.0;
	DamageValue td=damage;

	// Damage conversions
	for(int d=DPhysical; d<DNumTypes; ++d)
	{
		if(d!=damage.type_)
		{
			double convamount=GetStatValue(defender, DamageNames[damage.type_] + "TakenAs" + DamageNames[d]);
			if(convamount>0.0)
			{
				convamount=std::min(conversion,convamount); // Don't allow conversion to overflow for extra damage
				conversion-=convamount;
				DamageValue newd(d, td.value_ * convamount);
				td.value_ -= td.value_ * convamount;
				ConvertIncomingDamage(defender, out, newd, level-1);
			}
		}
	}

	//Extra damages
	for(int d=DPhysical; d<DNumTypes; ++d)
	{
		if(d!=damage.type_)
		{
			double a=GetStatValue(defender, DamageNames[damage.type_] + "TakenAsExtra" + DamageNames[d]);
			if(a>0.0)
			{
				DamageValue newd(d, td.value_ * a);
				ConvertIncomingDamage(defender, out, newd, level-1);
			}
		}
	}

	out.push_back(td);
}

DamageValueList BuildDamageList(StatSetCollection &attacker)
{
	// Collect flats, apply increases/decreases
	DamageRangeList flats;
	for(int i=DPhysical; i<DNumTypes; ++i)
	{
		// Get flat values
		std::string &dname=DamageNames[i];
		double low=GetStatValue(attacker, dname + "Low");
		double high=GetStatValue(attacker, dname + "High");

		if(high>0.0) flats.push_back(DamageRange(i,std::max(0.0,low),high));
	}

	// Roll damage values
	DamageValueList values;
	for(auto i : flats)
	{
		double val=rolld(i.low_, i.high_);
		values.push_back(DamageValue(i.type_, val));
	}
	// Add extra damage
	DamageValueList finalvalues;
	for(auto i : values)
	{
		ConvertDamage(attacker, finalvalues, i, 2);
	}

	return finalvalues;
}

DamageValueList BuildDamageList(StatSetCollection &attacker, DamageTypes type)
{
	std::string &dname=DamageNames[type];
	double low=GetStatValue(attacker, dname + "Low");
	double high=GetStatValue(attacker, dname + "High");

	DamageValueList values;
	double val=rolld(low, high);
	values.push_back(DamageValue(type, val));
	// Add extra damage
	DamageValueList finalvalues;
	for(auto i : values)
	{
		ConvertDamage(attacker, finalvalues, i, 2);
	}

	return finalvalues;
}

DamageValueList BuildDamageList(StatSetCollection &attacker, DamageRangeList &damage)
{
	DamageValueList values;
	for(auto i : damage)
	{
		double val=rolld(i.low_, i.high_);
		values.push_back(DamageValue(i.type_, val));
	}
	// Add extra damage
	DamageValueList finalvalues;
	for(auto i : values)
	{
		ConvertDamage(attacker, finalvalues, i, 2);
	}

	return finalvalues;
}

DamageValueList BuildDamageList(StatSetCollection &attacker, DamageValue &damage)
{
	DamageValueList finalvalues;
	ConvertDamage(attacker, finalvalues, damage, 2);
	return finalvalues;
}

DamageValueList ProcessIncomingDamage(StatSetCollection &defender, const DamageValueList &damages)
{
	DamageValueList dmg;
	for(auto i : damages)
	{
		ConvertIncomingDamage(defender, dmg, i, 2);
	}

	return dmg;
}


double ProcessHoT(StatSetCollection &attacker, double hot)
{
	double increased = 1.0 + GetStatValue(attacker, "IncreasedHealing");
	double more = 1.0 + GetStatValue(attacker,  "MoreHealing");

	increased -= GetStatValue(attacker, "DecreasedHealing");
	more *= 1.0 - GetStatValue(attacker, "LessHealing");

	return std::max(0.0, hot * increased * more);
}

double ProcessIncomingHoT(StatSetCollection &defender, double hot)
{
	double increased = 1.0 + GetStatValue(defender, "IncreasedHealingTaken");
	double more = 1.0 + GetStatValue(defender,  "MoreHealingTaken");

	increased -= GetStatValue(defender, "DecreasedHealingTaken");
	more *= 1.0 - GetStatValue(defender, "LessHealingTaken");

	return std::max(0.0, hot * increased * more);
}

bool MakeHitRoll(StatSetCollection &attacker, StatSetCollection &defender)
{
	// Calculate base chance to hit from level differentials
	double alev=GetStatValue(attacker, "Level");
	double dlev=GetStatValue(defender, "Level");

	// 5 over, 2 under
	double low=alev-2.0;
	double high=alev+5.0;
	double chance = 1.0-((dlev-low)/(high-low));

	// Clamp base chance
	chance=std::max(0.0, std::min(1.0, chance));
	//chance = 0.05 + chance * 90.0;

	// Apply increased chance to hit and chance to dodge
	double inchit=GetStatValue(attacker, "IncreasedChanceToHit");
	double dodge=GetStatValue(defender, "ChanceToDodge");

	chance = chance * (1.0 + (inchit-dodge));

	// Clamp to 5%,95%
	chance=std::max(0.0, std::min(1.0, chance));
	chance = 0.05 + chance * 0.90;
	// And roll
	return rolld(0,1) <= chance;
}

