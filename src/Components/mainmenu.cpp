#include "mainmenu.h"

#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Engine/Engine.h>

#include "../gamestatehandler.h"
#include "../scenetools.h"
#include "../playerdata.h"

#include "../Levels/mainmenustate.h"
#include "../Levels/testlevelstate.h"

void MainMenu::RegisterObject(Context *context)
{
	context->RegisterFactory<MainMenu>();
}

MainMenu::MainMenu(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void MainMenu::HandlePlay(StringHash eventType, VariantMap &eventData)
{
	auto pd=GetSubsystem<PlayerData>();
	auto gamestate=GetSubsystem<GameStateHandler>();

	//auto scene=CreateLevel(context_, "Areas/Test", 1, 0);
	//gamestate->SetState(scene);
	//gamestate->SwitchToLevel(1,0);
	SharedPtr<GameStateBase> level(new TestLevelState(context_));
	if(level)
	{
		level->GetData()[StringHash("level")]=1;
		level->GetData()[StringHash("previouslevel")]=0;
		pd->NewPlayer();
		pd->SetDungeonLevel(1);
		gamestate->SwitchToState(level);
	}

	element_->SetVisible(false);
}

void MainMenu::HandleExit(StringHash eventType, VariantMap &eventData)
{
	auto engine=GetSubsystem<Engine>();
	if(engine) engine->Exit();
}

void MainMenu::Update(float dt)
{
}

void MainMenu::DelayedStart()
{
	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();

	element_ = ui->LoadLayout(cache->GetResource<XMLFile>("UI/MainMenu.xml"));
	element_->SetPosition(IntVector2(graphics->GetWidth()/2 - element_->GetWidth()/2, graphics->GetHeight()/2-element_->GetHeight()/2));
	element_->SetVisible(true);
	ui->GetRoot()->AddChild(element_);

	element_->SetStyleAuto(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

	SubscribeToEvent(element_->GetChild("Play", true), StringHash("Pressed"), URHO3D_HANDLER(MainMenu, HandlePlay));
	SubscribeToEvent(element_->GetChild("Exit", true), StringHash("Pressed"), URHO3D_HANDLER(MainMenu, HandleExit));
}

void MainMenu::Stop()
{
	if(element_)
	{
		element_->Remove();
		element_.Reset();
	}
}
