#pragma once

// Hover handler
// Handle sending hover/unhover messages to whatever the mouse is pointing at or is no longer pointing at

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class HoverHandler : public LogicComponent
{
	URHO3D_OBJECT(HoverHandler, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	HoverHandler(Context *context);

	protected:
	virtual void Update(float dt) override;
	WeakPtr<Node> hovered_;
};
