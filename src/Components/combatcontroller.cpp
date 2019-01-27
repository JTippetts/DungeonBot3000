#include "combatcontroller.h"

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationEvents.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/Log.h>

void CombatController::RegisterObject(Context *context)
{
	context->RegisterFactory<CombatController>("Logic");

	URHO3D_ACCESSOR_ATTRIBUTE("Object Path", GetObjectPath, SetObjectPath, String, String(""), AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Animation Path", GetAnimPath, SetAnimPath, String, String(""), AM_DEFAULT);
}

CombatController::CombatController(Context *context) : LogicComponent(context), currentstate_(nullptr), nextstate_(nullptr)
{
	SetUpdateEventMask(USE_UPDATE);
}

void CombatController::DelayedStart()
{
	static StringHash CrowdAgentReposition("CrowdAgentNodeReposition");
	SubscribeToEvent(node_, CrowdAgentReposition, URHO3D_HANDLER(CombatController, HandleCrowdAgentReposition));
	SubscribeToEvent(node_, StringHash("AnimationTrigger"), URHO3D_HANDLER(CombatController, HandleAnimationTrigger));
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

bool CombatController::SetCombatActionState(CombatActionState *state)
{
	if(currentstate_)
	{
		if(currentstate_->IsLocking()) return false;   // Can't change state right now
	}
	nextstate_=state;
	return true;
}

void CombatController::Update(float dt)
{
	// State switch if necessary
	if(nextstate_)
	{
		if(nextstate_==currentstate_)
		{
			nextstate_=nullptr;
		}
		else
		{
			if(currentstate_) currentstate_->End(this);
			currentstate_=nextstate_;
			nextstate_=nullptr;
			currentstate_->Start(this);
		}
	}

	if(currentstate_)
	{
		CombatActionState *ns = currentstate_->Update(this,dt);
		if(ns)
		{
			if(SetCombatActionState(ns))
			{
			}
			else
			{
				Log::Write(LOG_ERROR, "Could not switch action state.");
			}
		}
	}


}

void CombatController::HandleAnimationTrigger(StringHash eventType, VariantMap &eventData)
{
	static StringHash Name("Name"), Data("Data");

	String animname=eventData[Name].GetString();
	unsigned int data=eventData[Data].GetUInt();
	if(currentstate_)
	{
		currentstate_->HandleTrigger(this, animname, data);
	}
}

void CombatController::HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData)
{
	using namespace CrowdAgentReposition;

	auto* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    auto* agent = static_cast<CrowdAgent*>(eventData[P_CROWD_AGENT].GetPtr());
    Vector3 velocity = eventData[P_VELOCITY].GetVector3();
    float timeStep = eventData[P_TIMESTEP].GetFloat();

	float speed = velocity.Length();

	if(speed>agent->GetRadius())
	{
		float speedRatio = speed / agent->GetMaxSpeed();
		node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, velocity), 10.0f * timeStep * speedRatio*1.0));
	}

	if(currentstate_)
	{
		currentstate_->HandleAgentReposition(this, velocity, timeStep);
	}
}

CombatActionState *CombatController::GetState(StringHash type)
{
	for (Vector<SharedPtr<CombatActionState> >::ConstIterator i = states_.Begin(); i != states_.End(); ++i)
    {
		if ((*i)->GetType() == type)
		{
			return *i;
		}
    }

	SharedPtr<CombatActionState> newComponent = DynamicCast<CombatActionState>(context_->CreateObject(type));
	if(newComponent) states_.Push(newComponent);
	return newComponent;
}
