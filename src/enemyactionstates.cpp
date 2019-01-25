#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/IO/Log.h>

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "Components/vitals.h"

#include "enemyactionstates.h"

CASEnemyUserIdle g_enemyuseridle;
CASEnemyUserChase g_enemyuserchase;

float rollf(float, float);

/////// Idle
CASEnemyUserIdle::CASEnemyUserIdle() : CombatActionState()
{
}

void CASEnemyUserIdle::Start(CombatController *actor)
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

void CASEnemyUserIdle::End(CombatController *actor)
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

CombatActionState *CASEnemyUserIdle::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() < 50)
	{
		return &g_enemyuserchase;
	}

	return nullptr;
}

void CASEnemyUserIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}
}

///////////////////
CASEnemyUserChase::CASEnemyUserChase() : CombatActionState()
{
}

void CASEnemyUserChase::End(CombatController *actor)
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

void CASEnemyUserChase::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
		}
	}
}

CombatActionState *CASEnemyUserChase::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() > 50)
	{
		return &g_enemyuseridle;
	}

	auto ev=node->GetComponent<EnemyVitals>();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ev && ca)
	{
		StatSetCollection ssc=ev->GetStats();
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
	}

	if(ca)
	{
		ca->SetTargetPosition(dudepos+Vector3(rollf(-8.0f,8.0f),0,rollf(-8.0f,8.0f)));
	}

	return nullptr;
}

void CASEnemyUserChase::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
}
