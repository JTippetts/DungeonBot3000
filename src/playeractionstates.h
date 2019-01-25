#pragma once

#include "combatactionstates.h"

// Player action states
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
