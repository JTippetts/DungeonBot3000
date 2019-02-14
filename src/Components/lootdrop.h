#pragma once

// Loot drop component
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class LootDrop : public LogicComponent
{
	URHO3D_OBJECT(LootDrop, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	LootDrop(Context *context);

	void SetMaxItems(unsigned int maxitems){maxitems_=maxitems;}
	const unsigned int &GetMaxItems() const {return maxitems_;}
	void SetDropChance(unsigned int chance){dropchance_=chance;}
	const unsigned int &GetDropChance() const {return dropchance_;}

	protected:
	unsigned int maxitems_;
	unsigned int dropchance_;

	virtual void DelayedStart() override;
	void HandleLifeDepleted(StringHash eventType, VariantMap &eventData);
};
