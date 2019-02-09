#include "gamestatehandler.h"

#include "Components/scenefader.h"

GameStateHandler::GameStateHandler(Context *context) : Object(context)
{
	SubscribeToEvent(StringHash("FadedOut"), URHO3D_HANDLER(GameStateHandler, HandleFadedOut));
}

void GameStateHandler::SetState(Scene *scene)
{
	if(!scene) return;

	if(currentscene_)
	{
		auto fader=currentscene_->GetComponent<SceneFader>();
		if(fader) fader->SetFadeState(SceneFader::FadingOut);
		nextscene_=scene;
	}
	else
	{
		currentscene_=scene;
		currentscene_->SetUpdateEnabled(true);
	}
}


void GameStateHandler::HandleFadedOut(StringHash eventType, VariantMap &eventData)
{
	if(currentscene_) currentscene_->Remove();
	currentscene_=nextscene_;
	nextscene_=nullptr;
	if(currentscene_) currentscene_->SetUpdateEnabled(true);
}
