#pragma once

// Life bubble UI display component
// Attaches to the player object to display a life bubble UI element

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

class LifeBubbleUI : public LogicComponent
{
	URHO3D_OBJECT(LifeBubbleUI, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	LifeBubbleUI(Context *context);

	protected:

	Material *healthmat_;
	SharedPtr<UIElement> element_;
	SharedPtr<Text> energyelement_;
	SharedPtr<Text> levelelement_;
	SharedPtr<UIElement> gamblebutton_;

	virtual void DelayedStart() override;
	virtual void Update(float dt) override;
	virtual void Stop() override;

	void HandleGamble(StringHash eventType, VariantMap &eventData);
};
