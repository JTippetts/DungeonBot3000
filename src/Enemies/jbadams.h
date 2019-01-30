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
};

