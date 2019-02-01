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
	URHO3D_ACCESSOR_ATTRIBUTE("AI State", GetAIState, SetAIState, String, String(""), AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Start State", GetStartState, SetStartState, String, String(""), AM_DEFAULT);
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

void CombatController::SetAIState(String s)
{
	if(s=="") return;
	aistate_=s;
	GetState(StringHash(s));
}

const String CombatController::GetAIState() const
{
	return aistate_;
}

void CombatController::SetStartState(String s)
{
	if(s=="") return;
	startstate_=s;
	SetCombatActionState(GetState(s));
}

const String CombatController::GetStartState() const
{
	return startstate_;
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
			//Log::Write(LOG_INFO, "No switch");
			nextstate_=nullptr;
		}
		else
		{
			if(currentstate_) currentstate_->End(this);
			currentstate_=nextstate_;
			nextstate_=nullptr;
			currentstate_->Start(this);
			//Log::Write(LOG_INFO, String("Switched to ") + currentstate_->GetTypeName());
		}
	}

	if(currentstate_)
	{
		nextstate_ = currentstate_->Update(this,dt);
		//if(nextstate_) Log::Write(LOG_INFO, String("Next state is: ") + nextstate_->GetTypeName());
	}

	cooldowns_.Tick(dt);
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
    float timeStep = eventData[P_TIMESTEP].GetFloat();
	auto agent=node_->GetComponent<CrowdAgent>();
	Vector3 velocity = agent->GetActualVelocity();

	if(currentstate_)
	{
		if(!currentstate_->HandleAgentReposition(this, velocity, timeStep)) FaceNodeMotion(timeStep);
	}
}

void CombatController::FaceNodeMotion(float timeStep)
{
    auto agent=node_->GetComponent<CrowdAgent>();
	Vector3 velocity = agent->GetActualVelocity();

	float speed = velocity.Length();

	if(speed>agent->GetRadius())
	{
		float speedRatio = speed / agent->GetMaxSpeed();
		node_->SetRotation(node_->GetRotation().Slerp(Quaternion(Vector3::FORWARD, velocity), 10.0f * timeStep * speedRatio*1.0));
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
	else Log::Write(LOG_INFO, "Unable to create state.");
	return newComponent;
}
