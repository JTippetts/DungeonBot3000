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
class CombatActionState
{
	public:
	virtual void Start(CombatController *actor)=0;
	virtual void End(CombatController *actor){};
	virtual CombatActionState *Update(CombatController *actor, float dt){return nullptr;};
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value){};
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt){};
	virtual bool IsLocking(){return false;}                   // Return true if the state can not be interrupted/changed
};

class CASPlayerIdle : public CombatActionState
{
	public:
	CASPlayerIdle();
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerMove : public CombatActionState
{
	public:
	CASPlayerMove();

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerSpinAttack : public CombatActionState
{
	public:
	CASPlayerSpinAttack();

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;
};

/// Instance some states to avoid lots of allocations
extern CASPlayerIdle g_playeridle;
extern CASPlayerMove g_playermove;
extern CASPlayerSpinAttack g_playerspinattack;

