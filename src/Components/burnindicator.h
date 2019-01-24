#pragma once

// Burn indicator

// Indicate if the owning unit is affected by any burns by showing a particle effect

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>

using namespace Urho3D;

class BurnIndicator : public LogicComponent
{
	URHO3D_OBJECT(BurnIndicator, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	BurnIndicator(Context *context);

	protected:
	virtual void DelayedStart() override;
	void HandleBurnPresent(StringHash eventType, VariantMap &eventData);

	SharedPtr<ParticleEmitter> emitter_;
};
