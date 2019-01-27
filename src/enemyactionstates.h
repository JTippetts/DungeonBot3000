#pragma once

// Enemy action states
#include "combatactionstates.h"

class CASEnemyIdle : public CombatActionState
{
	URHO3D_OBJECT(CASEnemyIdle, CombatActionState);
	public:
	CASEnemyIdle(Context *context);
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASEnemyChase : public CombatActionState
{
	URHO3D_OBJECT(CASEnemyChase, CombatActionState);
	public:
	CASEnemyChase(Context *context);
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASEnemyKick : public CombatActionState
{
	URHO3D_OBJECT(CASEnemyKick, CombatActionState);
	public:
	CASEnemyKick(Context *context);
	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;
};

/*
extern CASEnemyIdle g_enemyidle;
extern CASEnemyChase g_enemychase;
extern CASEnemyKick g_enemykick;
*/
