#include "timeddeath.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

TimedDeath::TimedDeath(Context *context) : Component(context), _ttl(1.0)
{
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(TimedDeath, HandlePostUpdate));
}

void TimedDeath::RegisterObject(Context *context)
{
	context->RegisterFactory<TimedDeath>("Timed");
	URHO3D_ATTRIBUTE("TTL", float, _ttl, 1.0f, AM_DEFAULT);
}

void TimedDeath::SetTTL(float ttl)
{
	_ttl=ttl;
}

void TimedDeath::HandlePostUpdate(StringHash eventType, VariantMap &eventData)
{
	static StringHash TimeStep("TimeStep"), Die("Die"), TriggerPayload("TriggerPayload");
	float dt=eventData[TimeStep].GetFloat();
	_ttl-=dt;
	if(_ttl <= 0.0f)
	{
		node_->SendEvent(TriggerPayload);
		node_->SendEvent(Die);
		node_->Remove();
	}
}
