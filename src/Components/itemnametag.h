#pragma once

// Item name tag
// Displays item name when on ground.
// Times out the item name display after a certain amount of time, reshows it while Alt is held.

// TODO: MAke a more sophisticated name tag management system to prevent overlaps of names

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class ItemNameTag : public LogicComponent
{
	URHO3D_OBJECT(ItemNameTag, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	ItemNameTag(Context *context);

	IntVector2 GetObjectScreenLocation();
	IntVector2 GetScreenLocation();
	IntVector2 GetScreenSize();

	void SetScreenLocation();
	void SetItemName(const String &name);
	void SetItemColor(const Color &c);
	UIElement *GetElement(){return element_;}
	void RemoveTag();

	protected:
	String name_;
	IntVector2 screenlocation_;
	SharedPtr<UIElement> element_;

	bool GetCollision(IntRect &rect, int &move);

	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update(float dt) override;
};
