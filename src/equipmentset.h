#pragma once

// Player equipment set
// Implement a class to maintain a set of equipped equipment
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>

#include "items.h"
#include <vector>

struct EquipmentEntry
{
	bool used_;
	bool active_;
	std::vector<EquipmentSlots> allowabletypes_;
	EquipmentItemDef def_;

	EquipmentEntry();
	EquipmentEntry(const String &allowables);

	bool Allowed(EquipmentSlots slot);
};

class EquipmentSet : public Object
{
	URHO3D_OBJECT(EquipmentSet, Object);
	public:
	EquipmentSet(Context *context);

	unsigned int CreateSlot(const String &allowables);
	bool CanAddItemToSlot(unsigned int slot, const EquipmentItemDef &def);
	bool AddItemToSlot(EquipmentItemDef &existing, unsigned int slot, const EquipmentItemDef &def);
	bool RemoveItemFromSlot(EquipmentItemDef &existing, unsigned int slot);
	bool GetItemAtSlot(EquipmentItemDef &existing, unsigned int slot);
	void SetSlotActive(unsigned int slot, bool a);

	StatSet *GetGlobalStats();
	StatSet *GetImplicitStats(unsigned int slot);
	unsigned int GetImplicitStats(StatSetCollection &ssc, EquipmentSlots eqtype);
	unsigned int GetNumberOfSlots();

	protected:
	std::vector<EquipmentEntry> slots_;
	StatSet globalstats_;
	std::vector<StatSet> localstats_;

	void RebuildGlobalStatSet();
};
