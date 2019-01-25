#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>

#include "../enemyactionstates.h"

// Enemy AI Base Component
// Implement a base component type (get derived AI component using Node::GetDerivedComponent) for enemy AI

using namespace Urho3D;

class EnemyAI : public Component
{
	URHO3D_OBJECT(EnemyAI, Component);
	public:
	static void RegisterObject(Context *context);
	EnemyAI(Context *context) : Component(context){}

	virtual CombatActionState *Callback(CombatActionState *state){return nullptr;};  // All-purpose callback for AI processing
};
