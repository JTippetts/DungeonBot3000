#pragma once

// Level changer
// Attach to a stairs object to change the level when used.

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class LevelChanger : public LogicComponent
{
	URHO3D_OBJECT(LevelChanger, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	LevelChanger(Context *context);

	void SetDestination(unsigned int level){destination_=level;}
	const unsigned int &GetDestination() const {return destination_;}

	void SetRadius(float radius){radius_=radius;}
	const float &GetRadius() const {return radius_;}

	void Use();
	protected:
	unsigned int destination_;
	float radius_;
};
