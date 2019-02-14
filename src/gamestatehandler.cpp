#include "gamestatehandler.h"

#include "Components/scenefader.h"
#include <Urho3D/IO/Log.h>

GameStateHandler::GameStateHandler(Context *context) : Object(context)
{
	SubscribeToEvent(StringHash("FadedOut"), URHO3D_HANDLER(GameStateHandler, HandleFadedOut));
	SubscribeToEvent(StringHash("EndFrame"), URHO3D_HANDLER(GameStateHandler, HandleEndFrame));
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
	if(currentscene_)
	{
		lastscene_=currentscene_;
		lastscene_->SetUpdateEnabled(false);
	}
	currentscene_=nextscene_;
	nextscene_=nullptr;
	if(currentscene_) currentscene_->SetUpdateEnabled(true);
}

void GameStateHandler::HandleEndFrame(StringHash eventType, VariantMap &eventData)
{
	//Log::Write(LOG_INFO, "End Frame");
	if(lastscene_)
	{
		lastscene_->Remove();
		lastscene_.Reset();
	}
}
