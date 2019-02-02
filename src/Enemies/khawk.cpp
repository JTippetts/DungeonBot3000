#include "khawk.h"

#include "KHawk.h"
#include "../Components/combatcontroller.h"
float rollf(float, float);

CASKHawkEnemyAI::CASKHawkEnemyAI(Context *context) : CASEnemyAI(context)
{
}

void CASKHawkEnemyAI::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
}

void CASKHawkEnemyAI::End(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
}

CombatActionState *CASKHawkEnemyAI::Update(CombatController *actor, float dt)
{
	auto node = actor->GetNode();

	auto pn = node->GetScene()->GetChild("Dude");

	Vector3 delta=pn->GetWorldPosition() - node->GetWorldPosition();
	float dist=delta.Length();

	float rl=rollf(0,100);
	if(rl<50)
	{
		CASEnemyApproachTarget *chase = actor->GetState<CASEnemyApproachTarget>();
		chase->SetApproachTarget(node->GetScene()->GetChild("Dude"));
		chase->SetApproachDistance(7.0);
		chase->SetTimeout(6.0);
		chase->SetApproachState(this);
		return chase;
	}
	else
	{
		CASEnemyMovePosition *move = actor->GetState<CASEnemyMovePosition>();
		auto navmesh = node->GetScene()->GetComponent<DynamicNavigationMesh>();
		Vector3 pt = navmesh->GetRandomPointInCircle(node->GetWorldPosition(), 20.0f);
		move->SetApproachPosition(pt);
		move->SetApproachDistance(3.0);
		move->SetTimeout(6.0);
		move->SetApproachState(this);
		return move;
	}
}
