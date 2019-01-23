#include "vitals.h"
#include <Urho3D/IO/Log.h>

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

		DamageValue val(DBurn, dot.dps_*mytime);
		DamageValueList dmg=BuildDamageList(dot.ownerstats_.collection_, val);
		ApplyDamageList(dmg);
		node_->SendEvent(DoTApplied, vm);
		if(erase) i=dots_.erase(i);
		else ++i;
	}
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

void PlayerVitals::ApplyDamageList(const DamageValueList &dmg)
{
	static StringHash LifeLow("LifeLow"), LifeDepleted("LifeDepleted"), DamageTaken("DamageTaken"), Damage("Damage");
	if(!vitalsstats_) return;
	VariantMap vm;

	DamageValueList actual=ProcessIncomingDamage(*vitalsstats_, dmg);

	double taken=0.0;

	for(auto i : actual)
	{
		taken += i.value_;
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

