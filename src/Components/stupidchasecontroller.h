#pragma once

// Stupid chase controller

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Navigation/CrowdAgent.h>

using namespace Urho3D;

float rollf(float,float);

class StupidChaseController : public LogicComponent
{
	URHO3D_OBJECT(StupidChaseController, LogicComponent);

	public:
	static void RegisterObject(Context *context)
	{
		context->RegisterFactory<StupidChaseController>("Logic");
	}

	StupidChaseController(Context *context) : LogicComponent(context)
	{
		SetUpdateEventMask(USE_UPDATE);
	}

	protected:
	void Update(float dt) override
	{
		auto *ca=node_->GetComponent<CrowdAgent>();
		ca->SetTargetPosition(node_->GetScene()->GetChild("Dude")->GetPosition()+Vector3(rollf(-8.0f,8.0f),0,rollf(-8.0f,8.0f)));
	}
};
