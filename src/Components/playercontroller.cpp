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
#include "../playeractionstates.h"

/*
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
}

void PlayerController::HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData)
{

}
*/
