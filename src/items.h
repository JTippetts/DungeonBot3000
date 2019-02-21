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
	EqShell,
	EqProcessor,
	EqDriveSystem,
	EqGeneratorSystem,
	EqShield,
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
	EquipmentItemDef(EquipmentSlots slot, ItemRarity rarity, String name, String dropobject, String object, const std::vector<String> &mods) : slot_(slot), rarity_(rarity), name_(name),
		dropobjectpath_(dropobject), objectpath_(object), itemmods_(mods){}

	EquipmentSlots slot_;                    // Slot type
	ItemRarity rarity_;
	String name_;
	String dropobjectpath_;
	String objectpath_;
	std::vector<String> itemmods_;
};


enum GeneralItemTypes
{
	GITEquipment,
	GITNumTypes
};

class GeneralItem : public Object
{
	URHO3D_OBJECT(GeneralItem, Object);
	public:
	GeneralItemTypes type_;
	IntVector2 invsize_;
	IntVector2 invlocation_;
	String inventoryimage_;
	EquipmentItemDef def_;

	GeneralItem(Context *context) : Object(context), type_(GITNumTypes){}
	GeneralItem(Context *context, const EquipmentItemDef &def) : Object(context), type_(GITEquipment), inventoryimage_(def.objectpath_), def_(def)
	{
		switch(def_.slot_)
		{
			{
				case EqBlade: invsize_=IntVector2(2,3); break;
				case EqTurret: invsize_=IntVector2(2,2); break;
				case EqShell: invsize_=IntVector2(2,3); break;
				case EqProcessor: invsize_=IntVector2(1,1); break;
				case EqDriveSystem: invsize_=IntVector2(2,2); break;
				case EqGeneratorSystem: invsize_=IntVector2(2,2); break;
				case EqShield: invsize_=IntVector2(2,3); break;
				default: invsize_=IntVector2(0,0); break;
			};
		}
	}
	GeneralItem(const GeneralItem &rhs) : Object(rhs.context_), type_(rhs.type_), invsize_(rhs.invsize_), inventoryimage_(rhs.inventoryimage_), def_(rhs.def_)
	{
	}
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
