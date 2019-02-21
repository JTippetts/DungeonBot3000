#pragma once
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/UIElement.h>
#include <vector>

#include "items.h"

using namespace Urho3D;
// Inventory screen

class InventoryScreen : public Object
{
	URHO3D_OBJECT(InventoryScreen, Object);
	public:
	InventoryScreen(Context *context);

	void Setup();
	void SetVisible(bool vis);
	bool IsVisible();

	void PutItemInHand(GeneralItem *item);
	bool HasItemInHand(GeneralItem *item);

	bool GetHoveredBagLocation(IntVector2 &location, const IntVector2 &mousepos);
	bool GetHoveredEquipmentSlot(unsigned int &slot, UIElement * &element, const IntVector2 &mousepos);

	protected:
	SharedPtr<UIElement> element_;
	UIElement *bagelement_;
	UIElement *bagimageelement_;
	UIElement *equipelement_;
	std::vector<UIElement *> bagslots_;
	HashMap<StringHash, UIElement *> equipslots_;
	unsigned int bagwidth_, bagheight_;
	SharedPtr<UIElement> hoveredelement_;

	void BuildSlot(StringHash name, unsigned int x, unsigned int y, unsigned int width, unsigned int height, const String &allowedtypes);
	void BuildBag(unsigned int width, unsigned int height);

	void UpdateEquipmentImages();
	void UpdateBagImages();

	void ResetBagColors();
	void HandleUpdate(StringHash eventType, VariantMap &eventData);
};
