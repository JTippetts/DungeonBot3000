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
