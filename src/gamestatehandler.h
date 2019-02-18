#pragma once

// Game state handler

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/BorderImage.h>

#include "gamestatebase.h"

using namespace Urho3D;

class GameStateHandler : public Object
{
	URHO3D_OBJECT(GameStateHandler, Object);
	public:
	GameStateHandler(Context *context);

	//void SwitchToMenu();
	//void SwitchToLevel(unsigned int level, unsigned int from);
	void SwitchToState(SharedPtr<GameStateBase> state);
	Scene* GetCurrentScene()
	{
		if(currentstate_) return currentstate_->GetScene();
		return nullptr;
	}

	protected:

	enum FadeState
	{
		FadingIn,
		FadedIn,
		FadingOut,
		FadedOut
	};
	//SharedPtr<Scene> currentscene_;
	SharedPtr<GameStateBase> currentstate_, nextstate_;

	SharedPtr<BorderImage> element_;
	FadeState fade_;
	float duration_, counter_;
	//unsigned int switchto_, from_;


	void HandleUpdate(StringHash eventType, VariantMap &eventData);
	void HandleEndFrame(StringHash eventType, VariantMap &eventData);
};
