#include "jbadams.h"
#include "../Components/combatcontroller.h"
float rollf(float, float);
CASjbadamsEnemyAI::CASjbadamsEnemyAI(Context *context) : CASEnemyAI(context)
{
}

void CASjbadamsEnemyAI::Start(CombatController *actor)
{
}

void CASjbadamsEnemyAI::End(CombatController *actor)
{
}

CombatActionState *CASjbadamsEnemyAI::Update(CombatController *actor, float dt)
{
	auto node = actor->GetNode();
	CASEnemyApproachTarget *chase = actor->GetState<CASEnemyApproachTarget>();
	chase->SetApproachTarget(node->GetScene()->GetChild("Dude"));
	chase->SetApproachDistance(7.0);
	chase->SetTimeout(0.0);

	float rl = rollf(0,100.0);
	if (rl < 50)
	{
		auto attack=actor->GetState<CASEnemyAttack>();
		attack->SetAttackStats(actor, "Objects/Mobs/jbadams/strike.json");
		attack->SetAnimation("/Models/Strike.ani");
		chase->SetApproachState(attack);
	}
	else
	{
		auto attack=actor->GetState<CASEnemyAttack>();
		attack->SetAttackStats(actor, "Objects/Mobs/jbadams/slam.json");
		attack->SetAnimation("/Models/Slam.ani");
		chase->SetApproachState(attack);
	}

	return chase;
}

