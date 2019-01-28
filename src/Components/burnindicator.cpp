#include "burnindicator.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/Log.h>


void BurnIndicator::RegisterObject(Context *context)
{
	context->RegisterFactory<BurnIndicator>("Logic");
}

BurnIndicator::BurnIndicator(Context *context) : LogicComponent(context)
{
}

void BurnIndicator::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("BurnsPresent"), URHO3D_HANDLER(BurnIndicator, HandleBurnPresent));

	auto cache=GetSubsystem<ResourceCache>();
	emitter_=node_->CreateComponent<ParticleEmitter>();
	auto pe = cache->GetResource<ParticleEffect>("Effects/inferno_blaze_particle.xml");
	if(pe)
	{
		emitter_->SetEffect(pe);
	}
	emitter_->SetEmitting(false);
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
