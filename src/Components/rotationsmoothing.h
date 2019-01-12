#pragma once

// Rotation smoothing for smooth turning of objects

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

class RotationSmoothing : public LogicComponent
{
	URHO3D_OBJECT(RotationSmoothing, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	RotationSmoothing(Context *context);

	virtual void Update(float dt) override;

	float GetRotationVelocity() const {return rotvel_;}
	void SetRotationVelocity(float v){rotvel_=v;}

	void SetAngle(float a);

	protected:
	float rotvel_, curangle_, nextangle_, lastuse_, lastusedelta_;


};
