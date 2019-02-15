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
			int rr=roll(1,100);
			ItemRarity ir=IRNormal;
			if(rr<4) ir=IRUnique;
			else if(rr<14) ir=IRRare;
			else if(rr<64) ir=IRMagic;

			int rt=roll(1,100);
			EquipmentSlots slot=EqBlade;
			if(rt<50) slot=EqTurret;

			EquipmentItemDef def;
			if(pd->GenerateRandomItem(def, slot, ir, pd->GetDungeonLevel())) pd->DropItem(def, node_->GetPosition(), node_->GetPosition()+Vector3(rollf(-5.0,5.0), 0, rollf(-5.0,5.0)));
		}
	}
}
