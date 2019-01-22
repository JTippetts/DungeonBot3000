#include "combatcontroller.h"

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationEvents.h>
#include <Urho3D/Graphics/AnimationController.h>

void CombatController::RegisterObject(Context *context)
{
	context->RegisterFactory<CombatController>("Logic");

	URHO3D_ACCESSOR_ATTRIBUTE("Object Path", GetObjectPath, SetObjectPath, String, String(""), AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Animation Path", GetAnimPath, SetAnimPath, String, String(""), AM_DEFAULT);
}

CombatController::CombatController(Context *context) : LogicComponent(context)
{
	static StringHash CrowdAgentReposition("CrowdAgentReposition");
	SetUpdateEventMask(USE_UPDATE);

	SubscribeToEvent(CrowdAgentReposition, URHO3D_HANDLER(CombatController, HandleCrowdAgentReposition));
}

void CombatController::SetObjectPath(String op)
{
	objectpath_=op;
}

const String CombatController::GetObjectPath() const
{
	return objectpath_;
}

void CombatController::SetAnimPath(String op)
{
	animpath_=op;
}

const String CombatController::GetAnimPath() const
{
	return animpath_;
}

void CombatController::MoveTo(Vector3 target)
{
	auto ca=node_->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(target);
	}
}

void CombatController::SetPushiness(NavigationPushiness pushy)
{
	auto ca=node_->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetNavigationPushiness(pushy);
	}
}

void CombatController::Update(float dt)
{
}

void CombatController::HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData)
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
        if (animCtrl->IsPlaying(animpath_ + "/Models/Walk.ani"))
        {
            float speedRatio = speed / agent->GetMaxSpeed();
            // Face the direction of its velocity but moderate the turning speed based on the speed ratio and timeStep
            node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, velocity), 10.0f * timeStep * speedRatio*0.0625));
            // Throttle the animation speed based on agent speed ratio (ratio = 1 is full throttle)
            animCtrl->SetSpeed(animpath_ + "/Models/Walk.ani", speedRatio * 0.25f);
        }
        else
		{
            animCtrl->Play(animpath_ + "/Models/Walk.ani", 0, true, 0.1f);
		}

        // If speed is too low then stop the animation
        if (speed < agent->GetRadius())
        {
			animCtrl->Stop(animpath_ + "/Models/Walk.ani", 0.5f);
			animCtrl->Play(animpath_ + "/Models/Idle.ani", 0, true, 0.5f);
		}
    }
}
