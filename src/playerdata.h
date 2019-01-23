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

	StatSetCollection &GetVitalsStats()  // Get the stat set collection pertinent to vitals.
	{
		vitalsstats_.clear();
		vitalsstats_.push_back(&basestats_);
		return vitalsstats_;
	}

	void LoadBaseStats(const String &name);

	protected:
	WeakPtr<Node> playernode_;

	StatSet basestats_;
	StatSetCollection vitalsstats_;
	StatModifierHandle levelmodifier_;

	ItemModTable itemmodtable_;
	ItemModTiers itemmodtiers_;

};
