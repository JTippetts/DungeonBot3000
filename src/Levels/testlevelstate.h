#pragma once

// Test level state

#include "../gamestatebase.h"

class TestLevelState : public GameStateBase
{
	URHO3D_OBJECT(TestLevelState, GameStateBase);
	public:
	TestLevelState(Context *context);

	virtual void Start() override;
};
