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
	URHO3D_ACCESSOR_ATTRIBUTE("Death Flash", GetDeathFlash, SetDeathFlash, String, String(""), AM_DEFAULT);
}

EnemyOnDeath::EnemyOnDeath(Context *context) : LogicComponent(context), removing_(false)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void EnemyOnDeath::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("LifeDepleted"), URHO3D_HANDLER(EnemyOnDeath, HandleLifeDepleted));
}

void EnemyOnDeath::SetDeathFlash(String path)
{
	deathflashpath_=path;
}

const String EnemyOnDeath::GetDeathFlash() const
{
	return deathflashpath_;
}

void EnemyOnDeath::HandleLifeDepleted(StringHash eventType, VariantMap &eventData)
{
	if(removing_) return;
	removing_=true;
	SubscribeToEvent(StringHash("EndFrame"), URHO3D_HANDLER(EnemyOnDeath, HandleEndFrame));

	// Spawn a death flash
	auto n=node_->GetScene()->CreateChild();
	n->SetScale(node_->GetScale());
	auto pe=n->CreateComponent<ParticleEmitter>();
	if(pe)
	{
		auto cache=GetSubsystem<ResourceCache>();
		String path="Effects/deathflash_particle.xml";
		if(deathflashpath_ != "") path=deathflashpath_;
		auto peff=cache->GetResource<ParticleEffect>(path);
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

