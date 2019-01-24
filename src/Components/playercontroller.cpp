#include "playercontroller.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationEvents.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include "thirdpersoncamera.h"
#include "vitals.h"
#include "../playerdata.h"
#include "combatcontroller.h"
#include "../combatactionstates.h"


void PlayerController::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerController>("Logic");
	URHO3D_ACCESSOR_ATTRIBUTE("Object Path", GetObjectPath, SetObjectPath, String, String(""), AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Animation Path", GetAnimPath, SetAnimPath, String, String(""), AM_DEFAULT);
}

PlayerController::PlayerController(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void PlayerController::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("AnimationTrigger"), URHO3D_HANDLER(PlayerController, HandleAnimationTrigger));
	SubscribeToEvent(node_, StringHash("CrowdAgentNodeReposition"), URHO3D_HANDLER(PlayerController, HandleCrowdAgentReposition));

	auto cc=node_->GetComponent<CombatController>();
	if(cc)
	{
		cc->SetCombatActionState(&g_playeridle);
	}
}

void PlayerController::SetObjectPath(String op)
{
	objectpath_=op;
}

const String PlayerController::GetObjectPath() const
{
	return objectpath_;
}

void PlayerController::SetAnimPath(String op)
{
	animpath_=op;
}

const String PlayerController::GetAnimPath() const
{
	return animpath_;
}

void PlayerController::HandleAnimationTrigger(StringHash eventType, VariantMap &eventData)
{
	static StringHash Name("Name"), Data("Data");

	if(eventData[Name].GetString()=="Spin")
	{
		PODVector<Node *> dudes;
		node_->GetScene()->GetChildrenWithComponent<EnemyVitals>(dudes, false);
		Vector3 mypos=node_->GetWorldPosition();

		for(auto i=dudes.Begin(); i!=dudes.End(); ++i)
		{
			Vector3 pos=(*i)->GetWorldPosition();
			Vector3 delta=mypos-pos;
			if(delta.Length() < 8)
			{
				auto myvitals = node_->GetComponent<PlayerVitals>();
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls && myvitals)
				{
					auto pd=GetSubsystem<PlayerData>();
					StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);
					vtls->ApplyDamageList(node_,ssc,dmg);
				}
			}
		}
	}

}

void PlayerController::Update(float dt)
{
	// Testing
	auto input=GetSubsystem<Input>();
	auto pd=GetSubsystem<PlayerData>();


	//StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");

	/*auto ca=node_->GetComponent<CrowdAgent>();
	if(ca)
	{
		auto cam=node_->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);
		auto animCtrl=node_->GetComponent<AnimationController>();

		if(input->GetMouseButtonDown(MOUSEB_LEFT) )
		{
			auto nav=node_->GetScene()->GetComponent<DynamicNavigationMesh>();

			ca->SetMaxSpeed(30.0f);
			Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
			ca->SetTargetPosition(gp);

			if(!animCtrl->IsPlaying(animpath_ + "/Models/Walk.ani"))
			{
				animCtrl->Play(animpath_ + "/Models/Walk.ani", 0, true, 0.1f);
			}
			if(animCtrl->IsPlaying(animpath_ + "/Models/Idle.ani"))
			{
				animCtrl->Stop(animpath_ + "/Models/Idle.ani", 0.1f);
			}
		}
		else
		{
			if(animCtrl->IsPlaying(animpath_ + "/Models/Walk.ani"))
			{
				animCtrl->Stop(animpath_ + "/Models/Walk.ani", 0.1f);
			}
			if(!animCtrl->IsPlaying(animpath_ + "/Models/Idle.ani"))
			{
				animCtrl->Play(animpath_ + "/Models/Idle.ani", 0, true, 0.1f);
			}
		}



		if(input->GetMouseButtonDown(MOUSEB_RIGHT) && input->GetMouseButtonDown(MOUSEB_LEFT))
		{
			auto ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
			double movespeed=GetStatValue(ssc, "MovementSpeed");
			double attackspeed=GetStatValue(ssc, "AttackSpeed");
			auto animCtrl=node_->GetComponent<AnimationController>();
			if(!animCtrl->IsPlaying(animpath_ + "/Models/Spin.ani"))
			{

				animCtrl->Play(animpath_ + "/Models/Spin.ani", 0, true, 0.1f);
			}
			animCtrl->SetSpeed(animpath_ + "/Models/Spin.ani", attackspeed);

			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
			ca->SetMaxSpeed(movespeed);
			//ca->SetTargetPosition(Vector3(ground.x_, 0, ground.y_));
		}
		else
		{
			auto ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "");
			double movespeed=GetStatValue(ssc, "MovementSpeed");
			auto animCtrl=node_->GetComponent<AnimationController>();
			if(animCtrl->IsPlaying(animpath_ + "/Models/Spin.ani"))
			{
				animCtrl->Stop(animpath_ + "/Models/Spin.ani");
			}
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetMaxSpeed(movespeed);
		}
	}*/
}

void PlayerController::HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData)
{

}
