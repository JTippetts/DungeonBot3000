#pragma once

// Player inventory
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/UIElement.h>
#include <vector>

#include "items.h"
#include "equipmentset.h"

class InventoryBag
{
	public:
	InventoryBag();

	void Setup(unsigned int width, unsigned int height);

	bool FindAvailableLocation(IntVector2 &location, const IntVector2 &size);
	bool FindAvailableLocation(IntVector2 &locatioin, GeneralItem *item);
	bool CanPlaceAtLocation(const IntVector2 &location, GeneralItem *item);
	void PlaceAtLocation(const IntVector2 &location, GeneralItem *item);
	GeneralItem *FindItemInSlot(const IntVector2 &location);
	void RemoveItem(GeneralItem *item);
	unsigned int CountCoveredItems(const IntVector2 &location, const IntVector2 &size);

	bool IsDirty();
	std::vector<WeakPtr<GeneralItem>> &GetItems(){return items_;}

	protected:
	std::vector<WeakPtr<GeneralItem>> items_;
	std::vector<bool> blocked_;
	unsigned int bagwidth_, bagheight_;
	bool dirty_;
};

class PlayerInventory : public Object
{
	URHO3D_OBJECT(PlayerInventory, Object);
	public:
	PlayerInventory(Context *context);

	bool CanEquipItemInSlot(unsigned int slot, GeneralItem *item, bool checkused=false);
	bool CanEquipItemInSlot(StringHash slot, GeneralItem *item, bool checkused=false);
	bool CanPlaceItemInBag(IntVector2 &pos, const IntVector2 &size);
	bool CanPlaceItemInBag(GeneralItem *item);

	bool AddItem(GeneralItem *item, bool equipifable=false);

	void AddEquipmentSlot(StringHash name, const String &allowed);
	bool GetSlot(StringHash name, unsigned int &slot);

	InventoryBag &GetBag();
	EquipmentSet &GetEquipment();

	protected:
	InventoryBag bag_;
	EquipmentSet equipment_;
	WeakPtr<GeneralItem> iteminhand_;
	HashMap<StringHash, unsigned int> equipmentslots_;
};
