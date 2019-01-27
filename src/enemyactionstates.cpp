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

	if(delta.Length() < 50)
	{
		auto ai=node->GetDerivedComponent<EnemyAI>();
		if(ai)
		{
			return ai->Callback(this);
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

///////////////////
CASEnemyChase::CASEnemyChase(Context *context) : CombatActionState(context)
{
}

void CASEnemyChase::End(CombatController *actor)
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

void CASEnemyChase::Start(CombatController *actor)
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
}

CombatActionState *CASEnemyChase::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();

	auto pl=node->GetScene()->GetChild("Dude");
	Vector3 pos=node->GetWorldPosition();
	Vector3 dudepos=pl->GetWorldPosition();
	Vector3 delta=dudepos-pos;

	if(delta.Length() > 50)
	{
		return actor->GetState<CASEnemyIdle>();
	}

	auto ca=node->GetComponent<CrowdAgent>();

	if(ca)
	{
		ca->SetTargetPosition(dudepos+Vector3(rollf(-8.0f,8.0f),0,rollf(-8.0f,8.0f)));
	}

	auto ai=node->GetDerivedComponent<EnemyAI>();
	if(ai)
	{
		auto ns = ai->Callback(this);
		if(ns) return ns;
	}

	return nullptr;
}

void CASEnemyChase::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
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
			return actor->GetState<CASEnemyIdle>();
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
