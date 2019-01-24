#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/IO/Log.h>

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "playerdata.h"
#include "Components/vitals.h"

CASPlayerIdle g_playeridle;
CASPlayerMove g_playermove;
CASPlayerSpinAttack g_playerspinattack;

/////// Idle
CASPlayerIdle::CASPlayerIdle() : CombatActionState()
{
}

void CASPlayerIdle::Start(CombatController *actor)
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
			//ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

void CASPlayerIdle::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
		}
	}
}

CombatActionState *CASPlayerIdle::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=node->GetSubsystem<Input>();

	if(input->GetMouseButtonPress(MOUSEB_RIGHT))
	{
		// Do right button
		//return nullptr;
		return &g_playerspinattack;
	}
	else
	{
		if(input->GetMouseButtonPress(MOUSEB_LEFT))
		{
			// Do walk
			return &g_playermove;
		}
	}

	return nullptr;
}

void CASPlayerIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}
}

////// PlayerMove
CASPlayerMove::CASPlayerMove() : CombatActionState()
{
}

void CASPlayerMove::Start(CombatController *actor)
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

void CASPlayerMove::End(CombatController *actor)
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

CombatActionState *CASPlayerMove::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();

	if(input->GetMouseButtonPress(MOUSEB_RIGHT))
	{
		return &g_playerspinattack;
	}
	else if(input->GetMouseButtonDown(MOUSEB_LEFT))
	{
		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
		ca->SetTargetPosition(gp);

		return nullptr;
	}
	else
	{
		// Not walking anymore, return idle state
		return &g_playeridle;
	}
}

void CASPlayerMove::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{

}


////////////// SpinAttack
CASPlayerSpinAttack::CASPlayerSpinAttack() : CombatActionState()
{
}

void CASPlayerSpinAttack::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
			ac->Play(actor->GetAnimPath() + "/Models/Spin.ani", 0, true, 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}
}

void CASPlayerSpinAttack::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani", 0.1f);
			ac->Stop(actor->GetAnimPath() + "/Models/Spin.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASPlayerSpinAttack::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();

	if(input->GetMouseButtonDown(MOUSEB_RIGHT))
	{
		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
		ca->SetTargetPosition(gp);

		double attackspeed=GetStatValue(ssc, "AttackSpeed");
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->SetSpeed(actor->GetAnimPath() + "/Models/Spin.ani", attackspeed);
		}

		return nullptr;
	}
	else
	{
		// Not walking anymore, return moving or idle state
		if(input->GetMouseButtonDown(MOUSEB_LEFT))
		{
			return &g_playermove;
		}

		return &g_playeridle;
	}
}

void CASPlayerSpinAttack::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	if(animname=="Spin")
	{
		auto node=actor->GetNode();
		PODVector<Node *> dudes;
		node->GetScene()->GetChildrenWithComponent<EnemyVitals>(dudes, false);
		Vector3 mypos=node->GetWorldPosition();

		for(auto i=dudes.Begin(); i!=dudes.End(); ++i)
		{
			Vector3 pos=(*i)->GetWorldPosition();
			Vector3 delta=mypos-pos;
			if(delta.Length() < 8)
			{
				auto myvitals = node->GetComponent<PlayerVitals>();
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls && myvitals)
				{
					auto pd=node->GetSubsystem<PlayerData>();
					StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);
					vtls->ApplyDamageList(node,ssc,dmg);
				}
			}
		}
	}
}
