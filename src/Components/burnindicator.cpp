#include "burnindicator.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>


void BurnIndicator::RegisterObject(Context *context)
{
	context->RegisterFactory<BurnIndicator>("Logic");
}

BurnIndicator::BurnIndicator(Context *context) : LogicComponent(context), emitter_(nullptr), ring_(nullptr)
{
	SetUpdateEventMask(USE_UPDATE);
}

void BurnIndicator::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("BurnsPresent"), URHO3D_HANDLER(BurnIndicator, HandleBurnPresent));
	SubscribeToEvent(node_, StringHash("HealsPresent"), URHO3D_HANDLER(BurnIndicator, HandleHealsPresent));

	auto cache=GetSubsystem<ResourceCache>();
	emitter_=node_->CreateComponent<ParticleEmitter>();
	auto pe = cache->GetResource<ParticleEffect>("Effects/inferno_blaze_particle.xml");
	if(pe)
	{
		emitter_->SetEffect(pe);
	}
	emitter_->SetEmitting(false);

	// Set up ring
	ring_ = node_->CreateChild();
	ring_->SetPosition(Vector3(0,1,0));
	ring_->SetScale(Vector3(4,1,4));
	auto mdl=ring_->CreateComponent<StaticModel>();
	mdl->SetModel(cache->GetResource<Model>("Effects/Ring.mdl"));
	mdl->SetMaterial(cache->GetResource<Material>("Effects/healindicator.xml"));
	mdl->SetEnabled(false);
}

void BurnIndicator::HandleBurnPresent(StringHash eventType, VariantMap &eventData)
{
	if(!emitter_) return;
	static StringHash Count("Count");

	unsigned int count=eventData[Count].GetUInt();

	if(count >0)
	{
		//Log::Write(LOG_INFO, "Burns present");
		emitter_->SetEmitting(true);
	}
	else emitter_->SetEmitting(false);
}

void BurnIndicator::HandleHealsPresent(StringHash eventType, VariantMap &eventData)
{
	if(!ring_) return;
	static StringHash Count("Count");

	unsigned int count=eventData[Count].GetUInt();
	if(count>0)
	{
		ring_->GetComponent<StaticModel>()->SetEnabled(true);
	}
	else ring_->GetComponent<StaticModel>()->SetEnabled(false);
}

void BurnIndicator::Update(float dt)
{

}
