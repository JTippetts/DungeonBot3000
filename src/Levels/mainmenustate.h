#pragma once

#include "../gamestatebase.h"

class MainMenuState : public GameStateBase
{
	URHO3D_OBJECT(MainMenuState, GameStateBase);
	public:
	MainMenuState(Context *context);

	virtual void Start() override;
};
