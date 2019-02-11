#pragma once

// Player Data Subsystem

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include "itemmods.h"
#include "items.h"
#include <unordered_map>
#include <vector>

using namespace Urho3D;

enum PlayerAttack
{
	PASpinAttack,
	PALaserBeam,
};

class PlayerData : public Object
{
	URHO3D_OBJECT(PlayerData, Object);

	public:
	PlayerData(Context *context);

	void LoadItemModTable(const String &name);
	ItemModTable &GetItemModTable(){return itemmodtable_;}

	void LoadItemModTiers(const String &name);
	ItemModTiers &GetItemModTiers(){return itemmodtiers_;}

	//StatSetCollection &GetStatSetCollection(int eqslot);

	StatSetCollection &GetVitalsStats()  // Get the stat set collection pertinent to vitals.
	{
		vitalsstats_.clear();
		vitalsstats_.push_back(&basestats_);
		for(unsigned int g=0; g<EqNumEquipmentSlots; ++g)
		{
			vitalsstats_.push_back(&equipmentglobalstats_[g]);
		}
		return vitalsstats_;
	}

	Scene *GetCurrentScene();
	Node *GetPlayerNode();

	void LoadBaseStats(const String &name);
	void LoadSkillStats(const String &name);

	StatSet *GetSkillStatSet(const std::string &name);

	StatSetCollection GetStatSetCollection(EquipmentSlots slot=EqNumEquipmentSlots, const std::string &skillname="");  // Pass EqNumEquipmentSlots and/or "" to disregard these parameters

	void EquipItem(const EquipmentItemDef &item, bool drop=false);
	void DropItem(const EquipmentItemDef &item, Vector3 dropperlocation, Vector3 location);

	void NewPlayer();
	void SpawnPlayer(Scene *scene, Vector3 location);  // Spawn player into current scene

	void SetEnergy(double e){energy_=e;}
	double GetEnergy(){return energy_;}

	PlayerAttack GetAttack(){return currentattack_;}
	void SetAttack(PlayerAttack a){currentattack_=a;}

	protected:
	StatSet basestats_;
	StatSetCollection vitalsstats_;
	StatModifierHandle levelmodifier_;

	EquipmentItemDef equipment_[EqNumEquipmentSlots];
	StatSet equipmentglobalstats_[EqNumEquipmentSlots];
	StatSet equipmentlocalstats_[EqNumEquipmentSlots];

	std::unordered_map<std::string, StatSet> skillstats_;

	ItemModTable itemmodtable_;
	ItemModTiers itemmodtiers_;

	double energy_;
	PlayerAttack currentattack_;

};
