#include "vitals.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>

void PlayerVitals::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerVitals>("Logic");
}

PlayerVitals::PlayerVitals(Context *context) : LogicComponent(context), vitalsstats_(nullptr), currentlife_(0), maximumlife_(0), energy_(0)
{
	SetUpdateEventMask(USE_UPDATE);
}

void PlayerVitals::SetStats(StatSetCollection *vitalsstats)
{
	vitalsstats_=vitalsstats;
	maximumlife_=GetStatValue(*vitalsstats_, "MaximumLife");
	currentlife_=maximumlife_;
}


double PlayerVitals::GetCurrentLife()
{
	return currentlife_;
}

double PlayerVitals::GetMaximumLife()
{
	return maximumlife_;
}

double PlayerVitals::GetEnergy()
{
	return 0;
}

void PlayerVitals::Update(float dt)
{
	if(!vitalsstats_) return;
	// First, update life
	maximumlife_=GetStatValue(*vitalsstats_, "MaximumLife");
	currentlife_=std::min(maximumlife_, currentlife_);

	// Apply over-time effects
	UpdateDoTs(dt);
	UpdateHoTs(dt);
}

void PlayerVitals::UpdateDoTs(float dt)
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
		BurnDoT &dot=i->second;
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
		if(currentlife_ <= 0.0) node_->SendEvent(LifeDepleted, vm);
		else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
		node_->SendEvent(DoTApplied, vm);
		if(erase) i=dots_.erase(i);
		else ++i;
	}

	unsigned int numdots=dots_.size();
	vm[Count]=numdots;
	node_->SendEvent(BurnsPresent, vm);
}

void PlayerVitals::UpdateHoTs(float dt)
{
	static StringHash LifeRegen("LifeRegen");
	// First do life regen tick
	double regen=GetStatValue(*vitalsstats_, "LifeRegen")*dt;
	double regenamt = ProcessHoT(*vitalsstats_, maximumlife_ * regen);
	//Log::Write(LOG_INFO, String("Life regen amt: ") + String(regenamt));
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

		// Todo, apply heal
		double amt=hot.hps_ * mytime;
		double actual=ProcessHoT(*vitalsstats_, amt);
		ApplyHealing(actual);
		node_->SendEvent(HoTApplied, vm);
		if(erase) i=hots_.erase(i);
		else ++i;
	}
}

void PlayerVitals::ApplyDamageList(Node *attackernode, const StatSetCollection &attackerstats, const DamageValueList &dmg)
{
	static StringHash LifeLow("LifeLow"), LifeDepleted("LifeDepleted"), DamageTaken("DamageTaken"), Damage("Damage");
	if(!vitalsstats_) return;
	VariantMap vm;

	DamageValueList actual=ProcessIncomingDamage(*vitalsstats_, dmg);

	double taken=0.0;

	for(auto i : actual)
	{
		if(i.type_==DBurn)
		{
			// Apply burn as a dot
			BurnDoT dot;
			dot.dps_=i.value_/4.0; // Value assumes per second
			double incdur=GetStatValue(attackerstats, "IncreasedBurnDuration");
			double reddur=GetStatValue(*vitalsstats_, "ReducedBurnDuration");
			double dur=4.0 * (1.0 + incdur - reddur);
			dot.ttl_=dur;
			dot.counter_=0.0;
			dot.owner_=attackernode;
			unsigned int id=attackernode->GetID();
			dots_[id]=dot;
		}
		else taken += i.value_;
	}
	currentlife_ -= taken;
	vm[Damage]=taken;
	node_->SendEvent(DamageTaken, vm);
	if(currentlife_ <= 0.0) node_->SendEvent(LifeDepleted, vm);
	else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
}

void PlayerVitals::ApplyHealing(double h)
{
	static StringHash HealingTaken("HealingTaken"), Healing("Healing");
	double actual = ProcessIncomingHoT(*vitalsstats_, h);
	VariantMap vm;

	vm[Healing]=actual;
	currentlife_ =std::min(maximumlife_, currentlife_+actual);
	node_->SendEvent(HealingTaken, vm);
}


///////////////////

void EnemyVitals::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyVitals>("Logic");
	URHO3D_ACCESSOR_ATTRIBUTE("Base Stats Filename", GetBaseStatsFilename, SetBaseStatsFilename, String, String(""), AM_DEFAULT);
}

EnemyVitals::EnemyVitals(Context *context) : LogicComponent(context), currentlife_(0), maximumlife_(0)
{
	SetUpdateEventMask(USE_UPDATE);

	basestatscollection_.push_back(&basestats_);
}

double EnemyVitals::GetCurrentLife()
{
	return currentlife_;
}

double EnemyVitals::GetMaximumLife()
{
	return maximumlife_;
}

void EnemyVitals::Update(float dt)
{
	maximumlife_=GetStatValue(basestatscollection_, "MaximumLife");
	currentlife_=std::min(maximumlife_, currentlife_);

	// Apply over-time effects
	UpdateDoTs(dt);
	UpdateHoTs(dt);
}

void EnemyVitals::UpdateDoTs(float dt)
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
		BurnDoT &dot=i->second;
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

		//Log::Write(LOG_INFO, String("dot: ") + String(dot.dps_) + " " + String(dot.ttl_) + " " + String(dot.counter_) + " " + String(mytime) + " " + String(dt));

		double taken=dot.dps_*mytime;
		currentlife_ -= taken;
		//Log::Write(LOG_INFO, String("Burn damage taken: ") + String(taken));
		vm[Damage]=taken;
		node_->SendEvent(DamageTaken, vm);
		if(currentlife_ <= 0.0) node_->SendEvent(LifeDepleted, vm);
		else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
		node_->SendEvent(DoTApplied, vm);

		if(erase) i=dots_.erase(i);
		else ++i;
	}

	unsigned int numdots=dots_.size();
	vm[Count]=numdots;
	node_->SendEvent(BurnsPresent, vm);
}

void EnemyVitals::UpdateHoTs(float dt)
{
	static StringHash LifeRegen("LifeRegen");
	// First do life regen tick
	double regen=GetStatValue(basestatscollection_, "LifeRegen")*dt;
	double regenamt = ProcessHoT(basestatscollection_, maximumlife_ * regen);
	//Log::Write(LOG_INFO, String("Life regen amt: ") + String(regenamt));
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

		// Todo, apply heal
		double amt=hot.hps_ * mytime;
		double actual=ProcessHoT(basestatscollection_, amt);
		ApplyHealing(actual);
		node_->SendEvent(HoTApplied, vm);
		if(erase) i=hots_.erase(i);
		else ++i;
	}
}

void EnemyVitals::ApplyDamageList(Node *attackernode, const StatSetCollection &attackerstats, const DamageValueList &dmg)
{
	static StringHash LifeLow("LifeLow"), LifeDepleted("LifeDepleted"), DamageTaken("DamageTaken"), Damage("Damage");
	VariantMap vm;

	DamageValueList actual=ProcessIncomingDamage(basestatscollection_, dmg);

	double taken=0.0;

	for(auto i : actual)
	{
		if(i.type_==DBurn)
		{
			// Apply burn as a dot
			//Log::Write(LOG_INFO, "Applying burn");
			BurnDoT dot;
			dot.dps_=i.value_ / 4.0;
			double incdur=GetStatValue(attackerstats, "IncreasedBurnDuration");
			double reddur=GetStatValue(basestatscollection_, "ReducedBurnDuration");
			double dur=4.0 * (1.0 + incdur - reddur);
			dot.ttl_=dur;
			dot.counter_=0.0;
			dot.owner_=attackernode;
			unsigned int id=attackernode->GetID();
			dots_[id]=dot;
		}
		else taken += i.value_;
	}
	currentlife_ -= taken;
	vm[Damage]=taken;
	node_->SendEvent(DamageTaken, vm);
	if(currentlife_ <= 0.0) node_->SendEvent(LifeDepleted, vm);
	else if(currentlife_ < maximumlife_ * 0.1) node_->SendEvent(LifeLow, vm);
}

void EnemyVitals::ApplyHealing(double h)
{
	static StringHash HealingTaken("HealingTaken"), Healing("Healing");
	double actual = ProcessIncomingHoT(basestatscollection_, h);
	VariantMap vm;

	vm[Healing]=actual;
	currentlife_ =std::min(maximumlife_, currentlife_+actual);
	node_->SendEvent(HealingTaken, vm);
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
