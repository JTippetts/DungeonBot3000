#pragma once

// Player Data Subsystem

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include "itemmods.h"


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

	void LoadItemModTable(const String &name);
	ItemModTable &GetItemModTable(){return itemmodtable_;}

	void LoadItemModTiers(const String &name);
	ItemModTiers &GetItemModTiers(){return itemmodtiers_;}

	StatSetCollection &GetStatSetCollection(int eqslot);

	protected:
	WeakPtr<Node> playernode_;

	StatSet basestats_;

	ItemModTable itemmodtable_;
	ItemModTiers itemmodtiers_;

};
