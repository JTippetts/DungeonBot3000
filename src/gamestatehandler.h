#pragma once

// Game state handler

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/BorderImage.h>

using namespace Urho3D;

class GameStateHandler : public Object
{
	URHO3D_OBJECT(GameStateHandler, Object);
	public:
	GameStateHandler(Context *context);

	void SwitchToMenu();
	void SwitchToLevel(unsigned int level, unsigned int from);
	Scene* GetCurrentScene(){return currentscene_;}

	protected:

	enum FadeState
	{
		FadingIn,
		FadedIn,
		FadingOut,
		FadedOut
	};
	SharedPtr<Scene> currentscene_;
	SharedPtr<BorderImage> element_;
	FadeState fade_;
	float duration_, counter_;
	unsigned int switchto_, from_;


	void HandleUpdate(StringHash eventType, VariantMap &eventData);
	void HandleEndFrame(StringHash eventType, VariantMap &eventData);
};
