#include "jbadams.h"
#include "../Components/combatcontroller.h"
float rollf(float, float);
CASjbadamsEnemyAI::CASjbadamsEnemyAI(Context *context) : CASEnemyAI(context)
{
}

void CASjbadamsEnemyAI::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
}

void CASjbadamsEnemyAI::End(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
}

CombatActionState *CASjbadamsEnemyAI::Update(CombatController *actor, float dt)
{
	static StringHash Leap("Leap");
	auto node = actor->GetNode();

	auto pn = node->GetScene()->GetChild("Dude");

	Vector3 delta=pn->GetWorldPosition() - node->GetWorldPosition();
	float dist=delta.Length();

	if(dist>15.0f && dist<60.0f && actor->GetCooldown(Leap) <= 0.0f)
	{
		actor->SetCooldown(Leap, 5.0f);
		auto leap = actor->GetState<CASjbadamsLeap>();
		leap->SetLeapTarget(pn->GetWorldPosition());
		return leap;
	}

	CASEnemyApproachTarget *chase = actor->GetState<CASEnemyApproachTarget>();
	chase->SetApproachTarget(node->GetScene()->GetChild("Dude"));
	chase->SetApproachDistance(7.0);
	chase->SetTimeout(6.0);

	float rl = rollf(0,100.0);
	if (rl < 50)
	{
		auto attack=actor->GetState<CASEnemyAttack>();
		attack->SetAttackStats(actor, "Objects/Mobs/jbadams/strike.json");
		attack->SetAnimation("/Models/Strike.ani");
		chase->SetApproachState(attack);
		attack->SetTargetNode(node->GetScene()->GetChild("Dude"));
	}
	else
	{
		auto attack=actor->GetState<CASEnemyAttackPosition>();
		attack->SetAttackStats(actor, "Objects/Mobs/jbadams/slam.json");
		attack->SetAnimation("/Models/Slam.ani");
		chase->SetApproachState(attack);
		attack->SetTargetNode(node->GetScene()->GetChild("Dude"));
		attack->SetTargetRadius(10.0f);
	}

	return chase;
}

CASjbadamsLeap::CASjbadamsLeap(Context *context) : CombatActionState(context)
{
}

void CASjbadamsLeap::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=actor->GetComponent<AnimationController>();
	auto ca=actor->GetComponent<CrowdAgent>();

	ca->SetEnabled(false);
	ac->Play(actor->GetAnimPath() + "/Models/Leap.ani", 0, false, 0.1f);

	time_=0.5f;
	startingpos_=node->GetWorldPosition();
	node->SetRotation(Quaternion(Vector3::FORWARD, leaptarget_-startingpos_));
}

void CASjbadamsLeap::End(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=actor->GetComponent<AnimationController>();
	auto ca=actor->GetComponent<CrowdAgent>();

	ca->SetEnabled(true);
	ac->Stop(actor->GetAnimPath() + "/Models/Leap.ani", 0.1f);
}

CombatActionState *CASjbadamsLeap::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	time_-=dt;
	float t=1.0f - (std::max(0.0f,time_)/0.5f);
	float vertscale=t*2.0f - 1.0f;
	vertscale *= vertscale;
	vertscale=1.0f-vertscale;
	Vector3 pos=startingpos_.Lerp(leaptarget_, t);
	pos.y_=vertscale*12.0f;

	node->SetWorldPosition(pos);

	if(time_<=0) return actor->GetDerivedState<CASEnemyAI>();
	else return nullptr;
}

