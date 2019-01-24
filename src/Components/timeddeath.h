#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/Component.h>

using namespace Urho3D;

class TimedDeath : public Component
{
	URHO3D_OBJECT(TimedDeath, Component)

	public:
	TimedDeath(Context *context);
	static void RegisterObject(Context *context);
	void SetTTL(float t);

	private:
	float _ttl;
	void HandlePostUpdate(StringHash eventType, VariantMap &eventData);
};

