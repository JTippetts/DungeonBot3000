#include "playercontroller.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationEvents.h>
#include <Urho3D/Graphics/AnimationController.h>
#include "thirdpersoncamera.h"
#include "vitals.h"
#include "../playerdata.h"


void PlayerController::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerController>("Logic");
	URHO3D_ACCESSOR_ATTRIBUTE("Object Path", GetObjectPath, SetObjectPath, String, String(""), AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Animation Path", GetAnimPath, SetAnimPath, String, String(""), AM_DEFAULT);
}

PlayerController::PlayerController(Context *context) : LogicComponent(context)
{
	static StringHash CrowdAgentReposition("CrowdAgentReposition");
	SetUpdateEventMask(USE_UPDATE);
	SubscribeToEvent(CrowdAgentReposition, URHO3D_HANDLER(PlayerController, HandleCrowdAgentReposition));
}

void PlayerController::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("AnimationTrigger"), URHO3D_HANDLER(PlayerController, HandleAnimationTrigger));
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

	//Log::Write(LOG_INFO, String("Triggered") + eventData[Name].GetString());
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
				/*DamageValueList dmg;
				DamageValue d1(DPhysical, 10.0);
				dmg.push_back(d1);*/
				auto myvitals = node_->GetComponent<PlayerVitals>();
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls && myvitals)
				{
					auto pd=GetSubsystem<PlayerData>();
					StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);

					for(auto d : dmg)
					{
						Log::Write(LOG_INFO, String(DamageNames[d.type_].c_str()) + ": " + String(d.value_));
					}

					vtls->ApplyDamageList(dmg);
				}
			}
		}
	}

}

void PlayerController::Update(float dt)
{
	// Testing
	auto input=GetSubsystem<Input>();
	if(input->GetKeyPress(KEY_A))
	{
		auto vitals=node_->GetComponent<PlayerVitals>();
		if(vitals)
		{
			DamageValueList dmg;
			DamageValue d1(DPhysical, 10.0);
			dmg.push_back(d1);
			vitals->ApplyDamageList(dmg);
		}
	}

	if(input->GetKeyPress(KEY_S))
	{
		// Test dealing damage to mobs
		PODVector<Node *> dudes;
		node_->GetScene()->GetChildrenWithComponent<EnemyVitals>(dudes, false);
		Vector3 mypos=node_->GetWorldPosition();

		for(auto i=dudes.Begin(); i!=dudes.End(); ++i)
		{
			Vector3 pos=(*i)->GetWorldPosition();
			Vector3 delta=mypos-pos;
			if(delta.Length() < 8)
			{
				/*DamageValueList dmg;
				DamageValue d1(DPhysical, 10.0);
				dmg.push_back(d1);*/
				auto myvitals = node_->GetComponent<PlayerVitals>();
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls && myvitals)
				{
					auto pd=GetSubsystem<PlayerData>();
					StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);

					for(auto d : dmg)
					{
						Log::Write(LOG_INFO, String(DamageNames[d.type_].c_str()) + ": " + String(d.value_));
					}

					vtls->ApplyDamageList(dmg);
				}
			}
		}
	}

	auto ca=node_->GetComponent<CrowdAgent>();
	if(ca)
	{
		auto cam=node_->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);
		if(input->GetMouseButtonDown(MOUSEB_LEFT) /*&& cam->PickGround(ground,mousepos.x_,mousepos.y_)*/)
		{
			ca->SetTargetPosition(Vector3(ground.x_, 0, ground.y_));
		}
		if(input->GetMouseButtonDown(MOUSEB_RIGHT))
		{
			auto animCtrl=node_->GetComponent<AnimationController>();
			if(!animCtrl->IsPlaying(animpath_ + "/Models/Spin.ani"))
			{
				animCtrl->Play(animpath_ + "/Models/Spin.ani", 0, true, 0.1f);
				animCtrl->SetSpeed(animpath_ + "/Models/Spin.ani", 4.0);
			}
		}
		else
		{
			auto animCtrl=node_->GetComponent<AnimationController>();
			if(animCtrl->IsPlaying(animpath_ + "/Models/Spin.ani"))
			{
				animCtrl->Stop(animpath_ + "/Models/Spin.ani");
			}
		}
	}
}

void PlayerController::HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData)
{
	using namespace CrowdAgentReposition;

	auto* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    auto* agent = static_cast<CrowdAgent*>(eventData[P_CROWD_AGENT].GetPtr());
    Vector3 velocity = eventData[P_VELOCITY].GetVector3();
    float timeStep = eventData[P_TIMESTEP].GetFloat();

	auto* animCtrl = node->GetComponent<AnimationController>();
    if (animCtrl)
    {
		if(animCtrl->IsPlaying(animpath_ + "/Models/Idle.ani"))
		{
			animCtrl->Stop(animpath_ + "/Models/Idle.ani", 0.5f);
		}

        float speed = velocity.Length(); // TODO
        if (animCtrl->IsPlaying(animpath_ + "/Models/Walk.ani") && !animCtrl->IsPlaying(animpath_ + "/Models/Spin.ani"))
        {
            float speedRatio = speed / agent->GetMaxSpeed();
            // Face the direction of its velocity but moderate the turning speed based on the speed ratio and timeStep
            //node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, velocity), 10.0f * timeStep * speedRatio*0.00625));
            // Throttle the animation speed based on agent speed ratio (ratio = 1 is full throttle)
            //animCtrl->SetSpeed(animpath_ + "/Models/Walk.ani", speedRatio * 0.25f);
        }
        else
		{
            animCtrl->Play(animpath_ + "/Models/Walk.ani", 0, true, 0.1f);
		}

        // If speed is too low then stop the animation
       /* if (speed < agent->GetRadius())
        {
			animCtrl->Stop(animpath_ + "/Models/Walk.ani", 0.5f);
			animCtrl->Play(animpath_ + "/Models/Idle.ani", 0, true, 0.5f);
		}*/
    }
}
