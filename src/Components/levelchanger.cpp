#include "levelchanger.h"

#include "../scenetools.h"
#include "../gamestatehandler.h"
#include "../playerdata.h"
#include "../Levels/testlevelstate.h"

#include <Urho3D/IO/Log.h>

void LevelChanger::RegisterObject(Context *context)
{
	context->RegisterFactory<LevelChanger>();
	URHO3D_ACCESSOR_ATTRIBUTE("Destination", GetDestination, SetDestination, unsigned int, 0, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Radius", GetRadius, SetRadius, float, 10.0f, AM_DEFAULT);
}

LevelChanger::LevelChanger(Context *context) : LogicComponent(context)
{
}

void LevelChanger::Use()
{
	auto pd=GetSubsystem<PlayerData>();
	auto gsh=GetSubsystem<GameStateHandler>();

	Log::Write(LOG_INFO, String("Changing from level ") + String(pd->GetDungeonLevel()) + " to " + String(destination_));
	//gsh->SetState(CreateLevel(context_, "Areas/test", destination_, pd->GetDungeonLevel()));
	//gsh->SwitchToLevel(destination_, pd->GetDungeonLevel());
	SharedPtr<GameStateBase> level(new TestLevelState(context_));
	if(level)
	{
		level->GetData()[StringHash("level")]=destination_;
		level->GetData()[StringHash("previouslevel")]=pd->GetDungeonLevel();
		gsh->SwitchToState(level);
		pd->SetDungeonLevel(destination_);
	}
}
