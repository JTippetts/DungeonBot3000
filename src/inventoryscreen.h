#pragma once

// Inventory screen

#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/UIElement.h>
#include <vector>

#include "items.h"
#include "equipmentset.h"

class InventoryScreen : public Object
{
	URHO3D_OBJECT(InventoryScreen, Object);
	public:
	InventoryScreen(Context *context);

	void Setup();
	void SetVisible(bool vis);
	bool IsVisible();

	bool CanEquipItemInSlot(unsigned int slot, EquipmentItemDef &def);
	UIElement *GetHoveredSlot(const IntVector2 &mousepos, unsigned int &slot);
	IntVector2 GetInventorySizeByType(EquipmentSlots slot);
	bool GetHoveredBagSlot(const IntVector2 &mousepos, IntVector2 &pos);
	GeneralItem *GetHoveredBagItem(const IntVector2 &mousepos, IntVector2 &slot);

	// Find a position available for an item of a given size
	bool FindBagPosition(IntVector2 &slotpos, const IntVector2 &size);

	protected:
	SharedPtr<UIElement> element_;
	UIElement *bag_;
	std::vector<UIElement *> bagslots_;
	std::vector<bool> bagslotsblocked_;
	unsigned int bagwidth_, bagheight_;
	std::vector<WeakPtr<GeneralItem>> itemsinbag_;

	std::vector<UIElement *> slots_;
	//std::vector<unsigned int> equipmentsetslots_;

	void BuildBag(unsigned int width, unsigned int height);
	void BuildSlot(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const String &allowedtypes);
	bool CanPlaceItemInSlot(const IntVector2 &pos, const IntVector2 &size, bool docolor=false);
	void ResetBagColors();
};
