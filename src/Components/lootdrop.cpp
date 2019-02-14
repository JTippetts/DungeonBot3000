#include "lootdrop.h"

#include "../playerdata.h"

int roll(int,int);
float rollf(float,float);

void LootDrop::RegisterObject(Context *context)
{
	context->RegisterFactory<LootDrop>();
	URHO3D_ACCESSOR_ATTRIBUTE("Max Items", GetMaxItems, SetMaxItems, unsigned int, 1, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Drop Chance", GetDropChance, SetDropChance, unsigned int, 50, AM_DEFAULT);
}

LootDrop::LootDrop(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_NO_EVENT);
}

void LootDrop::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("LifeDepleted"), URHO3D_HANDLER(LootDrop, HandleLifeDepleted));
}

void LootDrop::HandleLifeDepleted(StringHash eventType, VariantMap &eventData)
{
	int rl=roll(1,100);
	if(rl <= dropchance_)
	{
		int numdrops=roll(1,(int)maxitems_);
		auto pd=GetSubsystem<PlayerData>();
		for(int c=0; c<numdrops; c++)
		{
			EquipmentItemDef def;
			if(pd->GenerateRandomItem(def, EqBlade, IRRare, 10)) pd->DropItem(def, node_->GetPosition(), node_->GetPosition()+Vector3(rollf(-5.0,5.0), 0, rollf(-5.0,5.0)));
		}
	}
}
