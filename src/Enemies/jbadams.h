#pragma once

// jbadams boss-fight scripting

#include "../enemyactionstates.h"

class CASjbadamsEnemyAI : public CASEnemyAI
{
	URHO3D_OBJECT(CASjbadamsEnemyAI, CASEnemyAI);
	public:
	CASjbadamsEnemyAI(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;

	protected:
	float leapcooldown_;
};

class CASjbadamsLeap : public CombatActionState
{
	URHO3D_OBJECT(CASjbadamsLeap, CombatActionState);
	public:
	CASjbadamsLeap(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;

	void SetLeapTarget(Vector3 target){leaptarget_=target;}

	protected:
	Vector3 leaptarget_, startingpos_;
	float time_;
};
