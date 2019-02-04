#pragma once

// Item name tag container
// Manage item name tags to prevent overlap, manage picking an item name tag for player looting
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

#include "Components/itemnametag.h"
#include <vector>

using namespace Urho3D;

class ItemNameTagContainer : public Object
{
	URHO3D_OBJECT(ItemNameTagContainer, Object);
	public:
	ItemNameTagContainer(Context *context);

	void AddNameTag(ItemNameTag *tag);
	void RemoveNameTag(ItemNameTag *tag);

	ItemNameTag *GetHoveredTag();

	void DoItemHover();

	protected:
	std::vector<WeakPtr<ItemNameTag>> tags_;
};
