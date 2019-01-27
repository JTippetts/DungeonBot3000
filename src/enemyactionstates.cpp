#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/IO/Log.h>

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "Components/vitals.h"

#include "enemyactionstates.h"
#include "Components/enemyai.h"

#include "playerdata.h"

float rollf(float, float);

////////// Inactive
CASEnemyInactive::CASEnemyInactive(Context *context) : CombatActionState(context)
{
}

void CASEnemyInactive::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	auto ca=node->GetComponent<CrowdAgent>();

	ac->StopAll(0.1f);

	ca->SetTargetPosition(node->GetWorldPosition());
	ca->SetEnabled(false);
}

void CASEnemyInactive::End(CombatController *actor)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();

	ca->SetEnabled(true);
}

CombatActionState *CASEnemyInactive::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() < 60)
	{
		return actor->GetDerivedState<CASEnemyAI>();
	}
	return nullptr;
}

/////// Idle
CASEnemyIdle::CASEnemyIdle(Context *context) : CombatActionState(context)
{
}

void CASEnemyIdle::Start(CombatController *actor)
{

	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetTargetPosition(node->GetWorldPosition());
			ca->SetEnabled(false);
		}
	}
}

void CASEnemyIdle::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetEnabled(true);
		}
	}
}

CombatActionState *CASEnemyIdle::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() < 60)
	{
		//auto ai=node->GetDerivedComponent<EnemyAI>();
		auto ai=actor->GetDerivedState<CASEnemyAI>();
		if(ai)
		{
			//return ai->Callback(this);
			return ai;
		}
		else
		{
			Log::Write(LOG_ERROR, "No enemy AI in unit.");
		}
	}

	return nullptr;
}

void CASEnemyIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}
}

/////////////////// CASEnemyApproachTarget
CASEnemyApproachTarget::CASEnemyApproachTarget(Context *context) : CombatActionState(context), distance_(0), target_(nullptr), tostate_(nullptr), timeout_(0)
{
}

void CASEnemyApproachTarget::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani");
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

void CASEnemyApproachTarget::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
		}

		auto ev=node->GetComponent<EnemyVitals>();
		auto ca=node->GetComponent<CrowdAgent>();
		if(ev && ca)
		{
			StatSetCollection ssc=ev->GetStats();
			double movespeed=GetStatValue(ssc, "MovementSpeed");
			ca->SetMaxSpeed(movespeed*rollf(0.7,1.0));
		}
	}
	time_=0.0;
}

CombatActionState *CASEnemyApproachTarget::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() > 60)
	{
		//return actor->GetState<CASEnemyIdle>();
		return actor->GetState<CASEnemyInactive>();
	}

	if(delta.Length() <= distance_)
	{
		return tostate_;
	}

	if(timeout_>0.0)
	{
		time_+=dt;
		if(time_ > timeout_)
		{
			return actor->GetDerivedState<CASEnemyAI>();
		}
	}

	auto ca=node->GetComponent<CrowdAgent>();

	if(ca)
	{
		ca->SetTargetPosition(dudepos+Vector3(rollf(-8.0f,8.0f),0,rollf(-8.0f,8.0f)));
	}

	return nullptr;
}


void CASEnemyApproachTarget::SetApproachDistance(float dist)
{
	distance_=dist;
}

void CASEnemyApproachTarget::SetApproachState(CombatActionState *state)
{
	tostate_=state;
}

void CASEnemyApproachTarget::SetApproachTarget(Node *target)
{
	target_=target;
}
void CASEnemyApproachTarget::SetTimeout(float timeout)
{
	timeout_=timeout;
}


/////////////////////////
CASEnemyKick::CASEnemyKick(Context *context) : CombatActionState(context)
{
}

void CASEnemyKick::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Kick.ani", 0, false, 0.1f);
			auto vitals=node->GetComponent<EnemyVitals>();
			if(vitals)
			{
				auto ssc=vitals->GetStats();
				float attackspeed=std::max(0.01, GetStatValue(ssc, "AttackSpeed"));
				ac->SetSpeed(actor->GetAnimPath() + "/Models/Kick.ani", attackspeed);
			}
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}
}

void CASEnemyKick::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Kick.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASEnemyKick::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	if(ac)
	{
		if(ac->IsAtEnd(actor->GetAnimPath() + "/Models/Kick.ani"))
		{
			//return actor->GetState<CASEnemyIdle>();
			return actor->GetDerivedState<CASEnemyAI>();
		}
	}

	auto ca = node->GetComponent<CrowdAgent>();

	auto pd = node->GetSubsystem<PlayerData>();
	auto playerpos = pd->GetPlayerNode()->GetWorldPosition();
	auto delta = playerpos - node->GetWorldPosition();

	node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, delta), 10.0f * dt));

	return nullptr;
}

void CASEnemyKick::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	if(animname=="Kick")
	{
		auto node=actor->GetNode();
		auto vitals=node->GetComponent<EnemyVitals>();
		if(vitals)
		{
			auto pd=node->GetSubsystem<PlayerData>();
			auto pv=pd->GetPlayerNode()->GetComponent<PlayerVitals>();
			auto mystats=vitals->GetStats();
			DamageValueList dmg=BuildDamageList(mystats);
			if(pv)
			{
				pv->ApplyDamageList(node,mystats,dmg);
			}
		}
	}
}


/// User AI
CASUserEnemyAI::CASUserEnemyAI(Context *context) : CASEnemyAI(context)
{
}

 void CASUserEnemyAI::Start(CombatController *actor)
 {
 }

 void CASUserEnemyAI::End(CombatController *actor)
 {
 }

 CombatActionState *CASUserEnemyAI::Update(CombatController *actor, float dt)
 {
	auto node = actor->GetNode();
	CASEnemyApproachTarget *chase = actor->GetState<CASEnemyApproachTarget>();
	chase->SetApproachTarget(node->GetScene()->GetChild("Dude"));
	chase->SetApproachDistance(5.0);
	chase->SetTimeout(0.0);
	chase->SetApproachState(actor->GetState<CASEnemyKick>());

	return chase;
 }
