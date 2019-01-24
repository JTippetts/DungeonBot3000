#include "playercontroller.h"

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/UI/UI.h>
#include "thirdpersoncamera.h"
#include "vitals.h"


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
				DamageValueList dmg;
				DamageValue d1(DPhysical, 10.0);
				dmg.push_back(d1);
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls) vtls->ApplyDamageList(dmg);
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
	}
}
