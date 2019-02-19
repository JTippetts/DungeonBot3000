#pragma once

// Item name tag container
// Manage item name tags to prevent overlap, manage picking an item name tag for player looting
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>

#include <Urho3D/UI/UIElement.h>

#include "itemnametag.h"
#include <vector>

using namespace Urho3D;

class ItemNameTagContainer : public LogicComponent
{
	URHO3D_OBJECT(ItemNameTagContainer, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	ItemNameTagContainer(Context *context);

	void AddNameTag(ItemNameTag *tag);
	void RemoveNameTag(ItemNameTag *tag);

	ItemNameTag *GetHoveredTag();

	void DoItemHover();
	virtual void DelayedStart() override;
	virtual void Update(float dt) override;

	protected:
	SharedPtr<UIElement> layer_;
	std::vector<WeakPtr<ItemNameTag>> tags_;
	SharedPtr<UIElement> itemdesc_, equippeddesc_;

	void Populate();

	bool ready_;
};
