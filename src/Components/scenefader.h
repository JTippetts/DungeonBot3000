#pragma once

// Scene fader component
// Implement a fadein/fadeout effect using a full-screen UI quad
// Send an event when fully faded out or in
// Component starts faded out,

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/BorderImage.h>

using namespace Urho3D;

class SceneFader : public LogicComponent
{
	URHO3D_OBJECT(SceneFader, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	SceneFader(Context *context);

	enum States
	{
		FadingIn,
		FadedIn,
		FadingOut,
		FadedOut
	};

	void SetFadeDuration(float dur);
	void SetFadeState(States state);

	protected:
	virtual void Update(float dt) override;
	virtual void Start() override;
	float duration_, counter_;
	States state_;
	SharedPtr<BorderImage> quad_;
};
