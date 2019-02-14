#include "weaponequipper.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/AnimatedModel.h>

void WeaponEquipper::RegisterObject(Context *context)
{
	context->RegisterFactory<WeaponEquipper>();

	URHO3D_ACCESSOR_ATTRIBUTE("Bone Name", GetBoneName, SetBoneName, String, "", AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Object Name", GetObjectName, SetObjectName, String, "", AM_DEFAULT);
}

WeaponEquipper::WeaponEquipper(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void WeaponEquipper::DelayedStart()
{
	if(bonename_=="" || objectname_=="") return;

	auto scene=node_->GetScene();
	auto cache=GetSubsystem<ResourceCache>();
	auto mdl=node_->GetComponent<AnimatedModel>();
	if(!mdl) return;

	auto xfile=cache->GetResource<XMLFile>(objectname_);
	auto n=scene->InstantiateXML(xfile->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	auto rb=mdl->GetSkeleton().GetBone(bonename_);
	if(rb)
	{
		rb->node_->AddChild(n);
	}
}
