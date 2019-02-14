#pragma once

// Game state handler

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>

using namespace Urho3D;

class GameStateHandler : public Object
{
	URHO3D_OBJECT(GameStateHandler, Object);
	public:
	GameStateHandler(Context *context);

	void SetState(Scene *scene);
	Scene *GetCurrentScene(){return currentscene_;}

	protected:
	SharedPtr<Scene> currentscene_, nextscene_, lastscene_;

	void HandleFadedOut(StringHash eventType, VariantMap &eventData);
	void HandleEndFrame(StringHash eventType, VariantMap &eventData);
};
