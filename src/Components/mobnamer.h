#pragma once

// Mob namer
// Implement a class to randomize a mob's name
// Names are drawn from a set of internal lists, depending on the specified class
// Classes are Member (TODO), Moderator, and Emeritus

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class MobNamer : public LogicComponent
{
	URHO3D_OBJECT(MobNamer, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	MobNamer(Context *context);

	void SetClass(const String &c){class_=c;}
	const String &GetClass() const {return class_;}

	protected:
	String class_;

	virtual void DelayedStart() override;
};
