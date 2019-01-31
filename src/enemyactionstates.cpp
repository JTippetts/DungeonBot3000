#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/IO/Log.h>

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "Components/vitals.h"

#include "enemyactionstates.h"
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

	if(delta.Length() < 80)
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

bool CASEnemyIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}
	return true;
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
			StatSetCollection ssc=ev->GetVitalStats();
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
			//Log::Write(LOG_INFO, "1");
			ac->Play(actor->GetAnimPath() + "/Models/Kick.ani", 0, false, 0.1f);
			auto vitals=node->GetComponent<EnemyVitals>();
			if(vitals)
			{
				//Log::Write(LOG_INFO, "2");
				auto ssc=vitals->GetVitalStats();
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
			auto mystats=vitals->GetVitalStats();
			DamageValueList dmg=BuildDamageList(mystats);
			if(pv)
			{
				pv->ApplyDamageList(vitals,mystats,dmg);
			}
		}
	}
}

CASEnemyAttack::CASEnemyAttack(Context *context) : CombatActionState(context), target_(0)
{
}

void CASEnemyAttack::SetAnimation(const String a)
{
	animation_=a;
}

void CASEnemyAttack::SetAttackStats(CombatController *actor, const String a)
{
	attackstats_.Clear();
	auto cache=actor->GetSubsystem<ResourceCache>();
	auto file=cache->GetResource<JSONFile>(a);
	if(file)
	{
		attackstats_.LoadJSON(file->GetRoot());
	}
}

void CASEnemyAttack::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + animation_, 0, false, 0.1f);
			auto vitals=node->GetComponent<EnemyVitals>();
			if(vitals)
			{
				StatSetCollection ssc=vitals->GetVitalStats();
				ssc.push_back(&attackstats_);
				float attackspeed=std::max(0.01, GetStatValue(ssc, "AttackSpeed"));
				Log::Write(LOG_INFO, String("Attack speed: ") + String(attackspeed));
				ac->SetSpeed(actor->GetAnimPath() + animation_, attackspeed);
			}
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}
}

void CASEnemyAttack::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + animation_, 0.1f);
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

CombatActionState *CASEnemyAttack::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	if(ac)
	{
		if(ac->IsAtEnd(actor->GetAnimPath() + animation_))
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

void CASEnemyAttack::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	//if(animname=="Kick")
	{
		auto node=actor->GetNode();
		auto vitals=node->GetComponent<EnemyVitals>();
		if(vitals && target_)
		{
			//auto pd=node->GetSubsystem<PlayerData>();
			//auto pv=pd->GetPlayerNode()->GetComponent<PlayerVitals>();
			auto pv=target_->GetDerivedComponent<BaseVitals>();
			StatSetCollection mystats=vitals->GetVitalStats();
			mystats.push_back(&attackstats_);

			DamageValueList dmg=BuildDamageList(mystats);
			if(pv)
			{
				pv->ApplyDamageList(vitals,mystats,dmg);
			}
		}
	}
}

/////////////////////////
CASEnemyAttackPosition::CASEnemyAttackPosition(Context *context) : CombatActionState(context), targetcircle_(nullptr)
{
}

void CASEnemyAttackPosition::SetAnimation(const String a)
{
	animation_=a;
}

void CASEnemyAttackPosition::SetAttackStats(CombatController *actor, const String a)
{
	attackstats_.Clear();
	auto cache=actor->GetSubsystem<ResourceCache>();
	auto file=cache->GetResource<JSONFile>(a);
	if(file)
	{
		attackstats_.LoadJSON(file->GetRoot());
	}
}

void CASEnemyAttackPosition::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + animation_, 0, false, 0.1f);
			auto vitals=node->GetComponent<EnemyVitals>();
			if(vitals)
			{
				StatSetCollection ssc=vitals->GetVitalStats();
				ssc.push_back(&attackstats_);
				float attackspeed=std::max(0.01, GetStatValue(ssc, "AttackSpeed"));
				Log::Write(LOG_INFO, String("Attack speed: ") + String(attackspeed));
				ac->SetSpeed(actor->GetAnimPath() + animation_, attackspeed);
			}
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}

	if(target_) position_=target_->GetWorldPosition();

	if(!targetcircle_)
	{
		auto cache=node->GetSubsystem<ResourceCache>();

		targetcircle_ = node->CreateChild();
		auto md=targetcircle_->CreateComponent<StaticModel>();
		md->SetModel(cache->GetResource<Model>("Effects/Ring.mdl"));
		md->SetMaterial(cache->GetResource<Material>("Effects/targetring.xml"));
	}

	targetcircle_->SetWorldPosition(position_ + Vector3(0,0.3,0));
	targetcircle_->SetScale(Vector3(radius_, 1.0, radius_));
	targetcircle_->SetEnabled(true);
}

void CASEnemyAttackPosition::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + animation_, 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}

	target_=nullptr;
	targetcircle_->SetEnabled(false);
}

CombatActionState *CASEnemyAttackPosition::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto ac=node->GetComponent<AnimationController>();
	if(ac)
	{
		if(ac->IsAtEnd(actor->GetAnimPath() + animation_))
		{
			//return actor->GetState<CASEnemyIdle>();
			return actor->GetDerivedState<CASEnemyAI>();
		}
	}

	Vector3 dir=position_ - node->GetWorldPosition();

	node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, dir), 10.0f * dt));

	return nullptr;
}

void CASEnemyAttackPosition::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	//if(animname=="Kick")
	{
		auto node=actor->GetNode();
		auto vitals=node->GetComponent<EnemyVitals>();
		if(vitals && target_)
		{
			auto delta=target_->GetWorldPosition() - position_;
			if(delta.Length()<=radius_)
			{
				//auto pd=node->GetSubsystem<PlayerData>();
				//auto pv=pd->GetPlayerNode()->GetComponent<PlayerVitals>();
				auto pv=target_->GetDerivedComponent<BaseVitals>();
				StatSetCollection mystats=vitals->GetVitalStats();
				mystats.push_back(&attackstats_);

				DamageValueList dmg=BuildDamageList(mystats);
				if(pv)
				{
					pv->ApplyDamageList(vitals,mystats,dmg);
				}
			}
		}
	}
}

bool CASEnemyAttackPosition::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	return true;
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
