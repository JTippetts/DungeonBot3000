#include "enemymodifiers.h"
#include "enemynameplate.h"
#include "vitals.h"
#include <random>
#include <algorithm>

int roll(int low, int high);

void EnemyModifiers::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyModifiers>();
	URHO3D_ACCESSOR_ATTRIBUTE("Num Modifiers", GetNumModifiers, SetNumModifiers, unsigned int, 0, AM_DEFAULT);
}

EnemyModifiers::EnemyModifiers(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void EnemyModifiers::SetNumModifiers(const unsigned int num)
{
	nummodifiers_=num;
}

const unsigned int EnemyModifiers::GetNumModifiers() const
{
	return nummodifiers_;
}

void EnemyModifiers::DelayedStart()
{
	auto nameplate=node_->GetComponent<EnemyNameplate>();
	auto vitals=node_->GetComponent<EnemyVitals>();
	StatSet &stats=vitals->GetBaseStats();

	std::vector<unsigned int> mods={0,1,2,3,4,5,6,7};
	std::random_shuffle(mods.begin(), mods.end());

	for(unsigned int c=0; c<nummodifiers_; ++c)
	{
		int mod=mods[c];

		switch(mod)
		{
			case 0: // Reflect
			{
				nameplate->AddMod("Reflects Damage");
				stats.AddMod("ReflectAll", StatModifier::FLAT, "0.15");
			} break;
			case 1: // Berserk
			{
				nameplate->AddMod("Berserk");
				stats.AddMod("MovementSpeed", StatModifier::SCALE, "1.0");
				stats.AddMod("AttackSpeed", StatModifier::SCALE, "1.0");
			} break;
			case 2: // Regen life
			{
				nameplate->AddMod("Regenerates Life");
				stats.AddMod("LifeRegen", StatModifier::FLAT, "0.2");
			} break;
			case 3: // Extra life
			{
				nameplate->AddMod("Extra Life");
				stats.AddMod("MaximumLife", StatModifier::MULT, "0.5");
			} break;
			case 4: // Igniting
			{
				nameplate->AddMod("Ignites");
				stats.AddMod("PhysicalAsExtraBurn", StatModifier::FLAT, "1.0");
				stats.AddMod("FireAsExtraBurn", StatModifier::FLAT, "1.0");
				stats.AddMod("ElectricalAsExtraBurn", StatModifier::FLAT, "1.0");
			} break;
			case 5: // Quick
			{
				nameplate->AddMod("Quick");
				stats.AddMod("MovementSpeed", StatModifier::MULT, "0.5");
			} break;
			case 6: // Brutal
			{
				nameplate->AddMod("Brutal");
				stats.AddMod("AllDamage", StatModifier::SCALE, "0.4");
			} break;
			case 7: // Leech
			{
				nameplate->AddMod("Leeches Life");
				stats.AddMod("Leech", StatModifier::FLAT, "0.5");
			} break;
		};
	}
}
