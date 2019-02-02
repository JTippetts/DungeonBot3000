#pragma once

// KHawk boss-fight scripting

#include "../enemyactionstates.h"

class CASKHawkEnemyAI : public CASEnemyAI
{
	URHO3D_OBJECT(CASKHawkEnemyAI, CASEnemyAI);
	public:
	CASKHawkEnemyAI(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;

	protected:
};

class CASKHawkEnemyShootFire : public CombatActionState
{
	URHO3D_OBJECT(CASKHawkEnemyShootFire, CombatActionState);
	public:
	CASKHawkEnemyShootFire(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;

	void SetNumShots(int num);
	void SetTarget(Node *target);

	protected:
	WeakPtr<Node> target_;
	int numshots_;
	StatSet attackstats_;
	StatSetCollection stats_;
};
