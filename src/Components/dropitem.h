// Drop item
// Component to hold an item def information when dropped
#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "../items.h"

using namespace Urho3D;

class DropItemContainer : public LogicComponent
{
	URHO3D_OBJECT(DropItemContainer, LogicComponent);
	public:
	static void RegisterObject(Context *context);

	DropItemContainer(Context *context);
	//void SetItem(const EquipmentItemDef &item);
	//const EquipmentItemDef &GetItem() const;
	void SetItem(GeneralItem *item);
	GeneralItem *GetItem();

	protected:
	//EquipmentItemDef item_;
	WeakPtr<GeneralItem> item_;
};
