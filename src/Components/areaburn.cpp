#include "areaburn.h"
#include "vitals.h"

void AreaBurn::RegisterObject(Context *context)
{
	context->RegisterFactory<AreaBurn>();
}

AreaBurn::AreaBurn(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void AreaBurn::SetBurnAmount(float dmg)
{
	damage_=dmg;
}

void AreaBurn::SetInterval(float interval)
{
	interval_ = interval;
	counter_=0;
}

void AreaBurn::Update(float dt)
{
	if(counter_ <= 0.0f)
	{
		counter_=interval_;
		if(!owner_ || owner_.Expired()) return;
		PODVector<Node *> targets;
		BaseVitals *attackervitals=owner_->GetComponent<BaseVitals>();

		if(owner_.Get() == node_->GetScene()->GetChild("Dude"))
		{
			// Player burn effect
			node_->GetScene()->GetChildrenWithComponent<EnemyVitals>(targets, false);
		}
		else
		{
			node_->GetScene()->GetChildrenWithComponent<PlayerVitals>(targets, false);
		}

		for(auto i=targets.Begin(); i!=targets.End(); ++i)
		{
			Vector3 delta=(*i)->GetWorldPosition() - node_->GetWorldPosition();
			if(delta.Length() <= radius_)
			{
				DamageValueList dmg;
				dmg.push_back(DamageValue(DBurn, damage_));
				auto targetvitals = (*i)->GetComponent<BaseVitals>();
				if(targetvitals) targetvitals->ApplyDamageList(attackervitals, attackervitals->GetVitalStats(), dmg, false);
			}
		}
	}
	else
	{
		counter_ -= dt;
	}
}
