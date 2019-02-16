#include "playerdeath.h"
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Engine/Engine.h>

#include "combatcontroller.h"
#include "../gamestatehandler.h"
#include "../scenetools.h"
#include "../playerdata.h"


void PlayerDeath::RegisterObject(Context *context)
{
	context->RegisterFactory<PlayerDeath>();
}

PlayerDeath::PlayerDeath(Context *context) : LogicComponent(context), triggered_(false)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void PlayerDeath::HandleLifeDepleted(StringHash eventType, VariantMap &eventData)
{
	if(triggered_) return;
	triggered_=true;

	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();

	element_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/YouHaveDied.xml"));
	ui->GetRoot()->AddChild(element_);
	element_->SetPosition(IntVector2(graphics->GetWidth()/2-element_->GetWidth()/2, graphics->GetHeight()/2-element_->GetHeight()/2));

	SubscribeToEvent(element_->GetChild("Respawn",true), StringHash("Pressed"), URHO3D_HANDLER(PlayerDeath, HandleRespawn));
	SubscribeToEvent(element_->GetChild("Return",true), StringHash("Pressed"), URHO3D_HANDLER(PlayerDeath, HandleReturn));
	SubscribeToEvent(element_->GetChild("Exit",true), StringHash("Pressed"), URHO3D_HANDLER(PlayerDeath, HandleExit));
}

void PlayerDeath::HandleRespawn(StringHash eventType, VariantMap &eventData)
{
	auto pd=GetSubsystem<PlayerData>();
	auto gamestate=GetSubsystem<GameStateHandler>();

	element_->SetVisible(false);
	unsigned int level=pd->GetDungeonLevel();
	auto scene=CreateLevel(context_, "Areas/Test", level, level-1);
	gamestate->SetState(scene);
	//pd->SetDungeonLevel(1);

}

void PlayerDeath::HandleReturn(StringHash eventType, VariantMap &eventData)
{
	auto gamestate=GetSubsystem<GameStateHandler>();

	auto scene=CreateMainMenu(context_);
	gamestate->SetState(scene);
	element_->SetVisible(false);
}

void PlayerDeath::HandleExit(StringHash eventType, VariantMap &eventData)
{
	auto engine=GetSubsystem<Engine>();
	if(engine) engine->Exit();
}

void PlayerDeath::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("LifeDepleted"), URHO3D_HANDLER(PlayerDeath, HandleLifeDepleted));
}

void PlayerDeath::Stop()
{
	if(element_)
	{
		element_->Remove();
		element_.Reset();
	}
}
