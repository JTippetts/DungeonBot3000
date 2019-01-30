#pragma once

// Enemy on-death effects
// Perform the various jobs needed to clean up after an enemy dies


#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

class EnemyOnDeath : public LogicComponent
{
	URHO3D_OBJECT(EnemyOnDeath, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	EnemyOnDeath(Context *context);

	void SetDeathFlash(String s);
	const String GetDeathFlash() const;

	protected:
	void HandleEndFrame(StringHash eventType, VariantMap &eventData);
	void HandleLifeDepleted(StringHash eventType, VariantMap &eventData);
	virtual void DelayedStart() override;

	bool removing_;
	String deathflashpath_;
};
