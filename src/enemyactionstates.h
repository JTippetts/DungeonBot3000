#pragma once

// Enemy action states
#include "combatactionstates.h"

class CASEnemyUserIdle : public CombatActionState
{
	public:
	CASEnemyUserIdle();
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASEnemyUserChase : public CombatActionState
{
	public:
	CASEnemyUserChase();
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

extern CASEnemyUserIdle g_enemyuseridle;
extern CASEnemyUserChase g_enemyuserchase;
