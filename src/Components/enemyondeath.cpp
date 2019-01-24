#include "enemyondeath.h"

#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

#include "timeddeath.h"

void EnemyOnDeath::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyOnDeath>("Logic");
}

EnemyOnDeath::EnemyOnDeath(Context *context) : LogicComponent(context), removing_(false)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void EnemyOnDeath::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("LifeDepleted"), URHO3D_HANDLER(EnemyOnDeath, HandleLifeDepleted));
}

void EnemyOnDeath::HandleLifeDepleted(StringHash eventType, VariantMap &eventData)
{
	if(removing_) return;
	removing_=true;
	SubscribeToEvent(StringHash("EndFrame"), URHO3D_HANDLER(EnemyOnDeath, HandleEndFrame));

	// Spawn a death flash
	auto n=node_->GetScene()->CreateChild();
	auto pe=n->CreateComponent<ParticleEmitter>();
	if(pe)
	{
		auto cache=GetSubsystem<ResourceCache>();
		auto peff=cache->GetResource<ParticleEffect>("Effects/deathflash_particle.xml");
		pe->SetEffect(peff);
		pe->SetAutoRemoveMode(REMOVE_NODE);
	}
	//auto td=n->CreateComponent<TimedDeath>();
	//td->SetTTL(0.5);
	n->SetWorldPosition(node_->GetWorldPosition());
}

void EnemyOnDeath::HandleEndFrame(StringHash eventType, VariantMap &eventData)
{
	if(removing_) node_->Remove();
}

