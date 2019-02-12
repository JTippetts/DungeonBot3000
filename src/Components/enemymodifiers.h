#pragma once

// Enemy mods

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class EnemyModifiers : public LogicComponent
{
	URHO3D_OBJECT(EnemyModifiers, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	EnemyModifiers(Context *context);

	void SetNumModifiers(const unsigned int num);
	const unsigned int GetNumModifiers() const;

	protected:
	unsigned int nummodifiers_;

	virtual void DelayedStart() override;
};
