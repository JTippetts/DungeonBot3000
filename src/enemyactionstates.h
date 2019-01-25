#pragma once

// Enemy action states
#include "combatactionstates.h"

class CASEnemyIdle : public CombatActionState
{
	public:
	CASEnemyIdle();
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASEnemyChase : public CombatActionState
{
	public:
	CASEnemyChase();
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASEnemyKick : public CombatActionState
{
	public:
	CASEnemyKick();
	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;
};

extern CASEnemyIdle g_enemyidle;
extern CASEnemyChase g_enemychase;
extern CASEnemyKick g_enemykick;
