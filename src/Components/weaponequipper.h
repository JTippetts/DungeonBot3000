#pragma once

// Weapon Equipper
// Component to implement hooking a weapon to a skeleton bone

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class WeaponEquipper : public LogicComponent
{
	URHO3D_OBJECT(WeaponEquipper, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	WeaponEquipper(Context *context);

	void SetBoneName(const String &name){bonename_=name;}
	const String &GetBoneName() const {return bonename_;}
	void SetObjectName(const String &name){objectname_=name;}
	const String &GetObjectName() const {return objectname_;}

	protected:
	String bonename_, objectname_;

	virtual void DelayedStart() override;
};
