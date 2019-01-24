#pragma once

// Player Data Subsystem

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include "itemmods.h"
#include "items.h"
#include <unordered_map>
#include <vector>


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

	//StatSetCollection &GetStatSetCollection(int eqslot);

	StatSetCollection &GetVitalsStats()  // Get the stat set collection pertinent to vitals.
	{
		vitalsstats_.clear();
		vitalsstats_.push_back(&basestats_);
		return vitalsstats_;
	}

	void LoadBaseStats(const String &name);
	void LoadSkillStats(const String &name);

	StatSet *GetSkillStatSet(const std::string &name);

	StatSetCollection GetStatSetCollection(EquipmentSlots slot=EqNumEquipmentSlots, const std::string &skillname="");  // Pass EqNumEquipmentSlots and/or "" to disregard these parameters

	protected:
	WeakPtr<Node> playernode_;

	StatSet basestats_;
	StatSetCollection vitalsstats_;
	StatModifierHandle levelmodifier_;

	std::unordered_map<std::string, StatSet> skillstats_;

	ItemModTable itemmodtable_;
	ItemModTiers itemmodtiers_;

};
