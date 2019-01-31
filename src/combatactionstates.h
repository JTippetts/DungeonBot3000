#pragma once

// Combat Action States

// Implement an interface class that implements combat actions

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>

using namespace Urho3D;

class CombatController;
class CombatActionState : public Object
{
	URHO3D_OBJECT(CombatActionState, Object);
	public:
	CombatActionState(Context *context) : Object(context){}
	virtual void Start(CombatController *actor)=0;
	virtual void End(CombatController *actor){};
	virtual CombatActionState *Update(CombatController *actor, float dt){return nullptr;};
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value){};
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt){return false;};
	virtual bool IsLocking(){return false;}                   // Return true if the state can not be interrupted/changed
};


