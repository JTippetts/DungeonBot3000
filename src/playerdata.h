#pragma once

// Player Data Subsystem

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include "stats.h"

using namespace Urho3D;

class PlayerData : public Object
{
	URHO3D_OBJECT(PlayerData, Object);

	public:
	PlayerData(Context *context);

	void SetPlayerNode(Node *n)
	{
		playernode_=n;
	}
	Node *GetPlayerNode()
	{
		return playernode_.Get();
	}

	protected:
	WeakPtr<Node> playernode_;
};
