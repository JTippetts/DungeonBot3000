#include "khawk.h"

#include "KHawk.h"
#include "../Components/combatcontroller.h"
#include "../Components/vitals.h"

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
		auto shoot = actor->GetState<CASKHawkEnemyShootFire>();
		shoot->SetTarget(node->GetScene()->GetChild("Dude"));
		shoot->SetNumShots(5);
		move->SetApproachState(actor->GetState<CASKHawkEnemyShootFire>());
		return move;
	}
}


///////////// Launch Fire
CASKHawkEnemyShootFire::CASKHawkEnemyShootFire(Context *context) : CombatActionState(context)
{
}

void CASKHawkEnemyShootFire::Start(CombatController *actor)
{
	attackstats_.Clear();
	auto cache=actor->GetSubsystem<ResourceCache>();
	auto file=cache->GetResource<JSONFile>(actor->GetObjectPath() + "/launchfire.json");
	if(file)
	{
		attackstats_.LoadJSON(file->GetRoot());
	}

	auto node=actor->GetNode();
	auto vitals=node->GetComponent<EnemyVitals>();
	stats_ = vitals->GetVitalStats();
	stats_.push_back(&attackstats_);

	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			//Log::Write(LOG_INFO, "1");
			ac->Play(actor->GetAnimPath() + "/Models/Launch.ani", 0, true, 0.1f);

			if(vitals)
			{
				//Log::Write(LOG_INFO, "2");
				float attackspeed=std::max(0.01, GetStatValue(stats_, "AttackSpeed"));
				ac->SetSpeed(actor->GetAnimPath() + "/Models/Launch.ani", attackspeed);
			}
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}
}

void CASKHawkEnemyShootFire::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Launch.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}

	target_=nullptr;
}

CombatActionState *CASKHawkEnemyShootFire::Update(CombatController *actor, float dt)
{
	if(!target_ || target_.Expired())
	{
		return actor->GetDerivedState<CASEnemyAI>();
	}

	if(numshots_<=0)
	{
		return actor->GetDerivedState<CASEnemyAI>();
	}

	auto node=actor->GetNode();
	auto playerpos = target_->GetWorldPosition();
	auto delta = playerpos - node->GetWorldPosition();

	node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, delta), 10.0f * dt));

	return nullptr;
}

void CASKHawkEnemyShootFire::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	--numshots_;
}

void CASKHawkEnemyShootFire::SetNumShots(int num)
{
	numshots_=num;
}

void CASKHawkEnemyShootFire::SetTarget(Node *target)
{
	target_=target;
}

