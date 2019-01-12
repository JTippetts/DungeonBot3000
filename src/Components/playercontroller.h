#pragma once

// Player Controller

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

class PlayerController : public LogicComponent
{
	URHO3D_OBJECT(PlayerController, LogicComponent);

	public:
	static void RegisterObject(Context *context);
	PlayerController(Context *context);

	virtual void Update(float dt) override;

	protected:
};
