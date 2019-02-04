#include "burnpayload.h"

#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

#include "areaburn.h"


void BurnPayload::RegisterObject(Context *context)
{
	context->RegisterFactory<BurnPayload>();
}

BurnPayload::BurnPayload(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void BurnPayload::HandleTriggerPayload(StringHash eventType, VariantMap &eventData)
{
	auto n = node_->GetScene()->CreateChild();
	Vector3 pos = node_->GetWorldPosition();
	pos.y_=0.0f;

	auto cache = GetSubsystem<ResourceCache>();
	auto pe = cache->GetResource<ParticleEffect>("Effects/burnarea_particle.xml")->Clone();
	pe->SetActiveTime(duration_);
	auto emitter = n->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(pe);
	emitter->SetAutoRemoveMode(REMOVE_NODE);
	auto burner=n->CreateComponent<AreaBurn>();
	burner->SetBurnAmount(damage_);
	burner->SetInterval(interval_);
	burner->SetOwner(owner_);
	burner->SetRadius(radius_);

	n->SetWorldPosition(pos);
}

void BurnPayload::DelayedStart()
{
	static StringHash TriggerPayload("TriggerPayload");
	SubscribeToEvent(node_, TriggerPayload, URHO3D_HANDLER(BurnPayload, HandleTriggerPayload));
}

