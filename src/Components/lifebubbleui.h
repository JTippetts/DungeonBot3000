#pragma once

// Life bubble UI display component
// Attaches to the player object to display a life bubble UI element

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/UI/Sprite.h>
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

	SharedPtr<Scene> rttscene_;
	SharedPtr<Node> rttcameranode_;
	SharedPtr<Viewport> rttviewport_;
	SharedPtr<Material> rttmaterial_;
	SharedPtr<Texture2D> rttrendertexture_;
	Material *healthmat_;

	SharedPtr<Scene> overlayscene_;
	SharedPtr<Node> overlaynode_;
	SharedPtr<Viewport> overlayviewport_;

	SharedPtr<UIElement> element_;
	SharedPtr<Text> energyelement_;
	SharedPtr<UIElement> help_;
	SharedPtr<Text> levelelement_;
	SharedPtr<UIElement> gamblebutton_;

	virtual void DelayedStart() override;
	virtual void Update(float dt) override;
	virtual void Stop() override;

	void HandleGamble(StringHash eventType, VariantMap &eventData);
};
