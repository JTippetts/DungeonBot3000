#pragma once

// Equipment slots and items


// Thoughts
/*

Item mods: Name, StatSet

*/

#include "stats.h"
#include <vector>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/JSONValue.h>

#include "itemmods.h"

enum EquipmentSlots
{
	EqBlade,
	EqTurret,
	EqAntenna,
	EqProcessor,
	EqDriveSystem,
	EqGeneratorSystem,
	EqNumEquipmentSlots
};

enum ItemRarity
{
	IRNormal,
	IRMagic,
	IRRare,
	IRUnique
};

struct EquipmentItemDef
{
	EquipmentItemDef() : slot_(EqNumEquipmentSlots){}
	EquipmentItemDef(int slot, ItemRarity rarity, String name, String dropobject, String object, const std::vector<String> &mods) : slot_(slot), rarity_(rarity), name_(name),
		dropobjectpath_(dropobject), objectpath_(object), itemmods_(mods){}

	int slot_;                    // Slot type
	ItemRarity rarity_;
	String name_;
	String dropobjectpath_;
	String objectpath_;
	std::vector<String> itemmods_;
};


// Class to randomly generate an item from a list of mods.
// Mods are loaded from a table, and the tables are of the format
// [ [Weight, ModName] ]

class ItemWeightedModTable
{
	public:
	ItemWeightedModTable(){}

	int GetTableSize();
	void LoadJSON(const JSONValue &json);
	String Choose();

	protected:
	struct WeightedMod
	{
		double weight_;
		String modname_;
	};
	std::vector<WeightedMod> modtable_;
};

class ItemBuilder
{
	public:

	protected:

	struct WeightedMod
	{
		double weight_;
		String modname_;
	};


};
