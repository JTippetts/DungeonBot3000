#include "vitals.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>
#include "../playerdata.h"

void BaseVitals::RegisterObject(Context *context)
{

}

BaseVitals::BaseVitals(Context *context) : LogicComponent(context), currentlife_(0), maximumlife_(0)
{
	SetUpdateEventMask(USE_UPDATE);
}

double BaseVitals::GetCurrentLife()
{
	return currentlife_;
}

double BaseVitals::GetMaximumLife()
{
	return maximumlife_;
}

void BaseVitals::Update(float dt)
{
	auto vitalstats=GetVitalStats();
	// First, update life
	maximumlife_=GetStatValue(vitalstats, "MaximumLife");
	currentlife_=std::min(maximumlife_, currentlife_);

	// Apply over-time effects
	UpdateDoTs(dt);
	UpdateHoTs(dt);
}

void BaseVitals::DelayedStart()
{
	auto vitalstats=GetVitalStats();
	maximumlife_=GetStatValue(vitalstats, "MaximumLife");
	currentlife_=maximumlife_;
}

void BaseVitals::UpdateDoTs(float dt)
{
	static StringHash DoTExpiring("DoTExpiring"), DoTApplied("DoTApplied"), Attacker("Attacker");
	static StringHash LifeLow("LifeLow"), LifeDepleted("LifeDepleted"), DamageTaken("DamageTaken"), Damage("Damage");
	static StringHash BurnsPresent("BurnsPresent"), Count("Count");
	VariantMap vm;

	auto i=dots_.begin();
	while(i != dots_.end())
	{
		bool erase=false;
		double mytime=dt;
		BurnDoT &dot=(*i);//i->second;
		vm[Attacker]=dot.owner_;
		if(dot.ttl_ != -1)
		{
			if(dot.counter_ + mytime > dot.ttl_)
			{
				mytime=dot.ttl_ - dot.counter_;
				node_->SendEvent(DoTExpiring, vm);
				erase=true;
			}
			else
			{
				dot.counter_+=mytime;
			}
		}

		double taken=dot.dps_*mytime;
		currentlife_ -= taken;
		vm[Damage]=taken;
		node_->SendEvent(DamageTaken, vm);
		if(currentlife_ <= 0.0)
		{
			node_->SendEvent(LifeDepleted, vm);
			return;
		}
		else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
		node_->SendEvent(DoTApplied, vm);
		if(erase) i=dots_.erase(i);
		else ++i;
	}

	unsigned int numdots=dots_.size();
	vm[Count]=numdots;
	node_->SendEvent(BurnsPresent, vm);
}

void BaseVitals::ApplyHoT(double h, double ttl)
{
	HealHoT hot;
	hot.ttl_=ttl;
	hot.hps_=h;
	hot.counter_=0.0;

	hots_.push_back(hot);
}

void BaseVitals::UpdateHoTs(float dt)
{
	static StringHash LifeRegen("LifeRegen");
	// First do life regen tick
	auto vitalstats=GetVitalStats();
	double regen=GetStatValue(vitalstats, "LifeRegen")*dt;
	double regenamt = ProcessHoT(vitalstats, maximumlife_ * regen);
	ApplyHealing(regenamt);

	// Now do hots
	static StringHash HoTExpiring("HoTExpiring"), HoTApplied("HoTApplied"), Attacker("Attacker");
	VariantMap vm;

	auto i=hots_.begin();
	while(i != hots_.end())
	{
		bool erase=false;
		double mytime=dt;
		HealHoT &hot=*i;
		if(hot.ttl_ != -1)
		{
			if(hot.counter_ + mytime > hot.ttl_)
			{
				mytime=hot.ttl_ - hot.counter_;
				node_->SendEvent(HoTExpiring, vm);
				erase=true;
			}
			else
			{
				hot.counter_+=mytime;
			}
		}

		double amt=hot.hps_ * mytime;
		double actual=ProcessHoT(vitalstats, amt);
		ApplyHealing(actual);
		node_->SendEvent(HoTApplied, vm);
		if(erase) i=hots_.erase(i);
		else ++i;
	}
}

void BaseVitals::ApplyDamageList(BaseVitals *attackervitals, const StatSetCollection &attackerstats, const DamageValueList &dmg, bool reflectable)
{
	static StringHash LifeLow("LifeLow"), LifeDepleted("LifeDepleted"), DamageTaken("DamageTaken"), Damage("Damage");
	auto vitalstats=GetVitalStats();
	VariantMap vm;
	auto attackernode=attackervitals->GetNode();

	DamageValueList actual=ProcessIncomingDamage(vitalstats, dmg);

	double taken=0.0;

	DamageValueList reflected;

	for(auto i : actual)
	{
		if(i.type_==DBurn)
		{
			// Apply burn as a dot
			BurnDoT dot;
			dot.dps_=i.value_/4.0; // Value assumes per second
			double incdur=GetStatValue(attackerstats, "IncreasedBurnDuration");
			double reddur=GetStatValue(vitalstats, "ReducedBurnDuration");
			double dur=4.0 * (1.0 + incdur - reddur);
			dot.ttl_=dur;
			dot.counter_=0.0;
			dot.owner_=attackernode;
			unsigned int id=attackernode->GetID();
			//dots_[id]=dot;
			dots_.push_back(dot);
		}
		else
		{
			taken += i.value_;

			// Calculate reflected damage
			double reflect=GetStatValue(vitalstats, "Reflect" + DamageNames[i.type_]);
			if(i.type_==DFire || i.type_==DBurn || i.type_==DElectrical) reflect += GetStatValue(vitalstats, "ReflectElemental");
			reflect += GetStatValue(vitalstats, "ReflectAll");
			reflect = std::min(1.0, reflect); // Can't reflect more than 100%

			if(reflect>0.0 && reflectable)
			{
				DamageValue ref(i.type_, i.value_ * reflect);
				reflected.push_back(ref);
				Log::Write(LOG_INFO, String("Reflecting: ") + String(ref.value_) + " " + String(DamageNames[i.type_].c_str()));
			}
		}

	}
	currentlife_ -= taken;
	vm[Damage]=taken;
	node_->SendEvent(DamageTaken, vm);

	// Send back any leach
	double leech=GetStatValue(attackerstats, "Leech");
	if(leech>0.0)
	{
		attackervitals->ApplyHoT(taken * leech, 4.0);
	}

	// Send back reflected
	if(reflected.size()>0 && reflectable)
	{
		attackervitals->ApplyDamageList(this, vitalstats, reflected, false);
	}

	if(currentlife_ <= 0.0)
	{
		node_->SendEvent(LifeDepleted, vm);
		return;
	}
	else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
}

void BaseVitals::ApplyHealing(double h)
{
	static StringHash HealingTaken("HealingTaken"), Healing("Healing");
	auto vitalstats=GetVitalStats();
	double actual = ProcessIncomingHoT(vitalstats, h);
	VariantMap vm;

	vm[Healing]=actual;
	currentlife_ =std::min(maximumlife_, currentlife_+actual);
	node_->SendEvent(HealingTaken, vm);
}

//////////////// Player

void PlayerVitals::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerVitals>();
}

PlayerVitals::PlayerVitals(Context *context) : BaseVitals(context)
{
}

const StatSetCollection &PlayerVitals::GetVitalStats() const
{
	auto pd=GetSubsystem<PlayerData>();
	return pd->GetVitalsStats();
}

void PlayerVitals::Update(float dt)
{
	BaseVitals::Update(dt);
	auto vitalstats=GetVitalStats();
	auto pd=GetSubsystem<PlayerData>();

	double energygen=GetStatValue(vitalstats, "EnergyGen");
	double energy=pd->GetEnergy();
	energy += energygen * (double)dt;
	pd->SetEnergy(energy);
}
///////////////////

void EnemyVitals::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyVitals>("Logic");
	URHO3D_ACCESSOR_ATTRIBUTE("Base Stats Filename", GetBaseStatsFilename, SetBaseStatsFilename, String, String(""), AM_DEFAULT);
}

EnemyVitals::EnemyVitals(Context *context) : BaseVitals(context)
{
	SetUpdateEventMask(USE_UPDATE);

	basestatscollection_.push_back(&basestats_);
}

void EnemyVitals::SetBaseStatsFilename(const String &filename)
{
	if(filename=="") return;

	basestatsfilename_=filename;
	auto cache=GetSubsystem<ResourceCache>();
	auto file=cache->GetResource<JSONFile>(filename);
	if(file)
	{
		basestats_.LoadJSON(file->GetRoot());
	}

	maximumlife_=GetStatValue(basestatscollection_, "MaximumLife");
	currentlife_=maximumlife_;
}

const String EnemyVitals::GetBaseStatsFilename() const
{
	return basestatsfilename_;
}

void EnemyVitals::SetLevel(unsigned int level)
{
	basestats_.AddMod("Level", StatModifier::FLAT, std::to_string(level));
	maximumlife_=GetStatValue(basestatscollection_, "MaximumLife");
	currentlife_=maximumlife_;
}
