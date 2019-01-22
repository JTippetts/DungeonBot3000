#include "playercontroller.h"

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/UI.h>
#include "thirdpersoncamera.h"


void PlayerController::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerController>("Logic");
}

PlayerController::PlayerController(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void PlayerController::Update(float dt)
{
	auto ca=node_->GetComponent<CrowdAgent>();
	if(ca)
	{
		auto input=GetSubsystem<Input>();
		auto cam=node_->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);
		if(input->GetMouseButtonDown(MOUSEB_LEFT) /*&& cam->PickGround(ground,mousepos.x_,mousepos.y_)*/)
		{
			ca->SetTargetPosition(Vector3(ground.x_, 0, ground.y_));
		}
	}
}
