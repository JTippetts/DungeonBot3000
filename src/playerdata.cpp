#include "playerdata.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>

#include "Components/dropitem.h"
#include "Components/itemnametag.h"
#include "Components/vitals.h"
#include "gamestatehandler.h"

#include "playerinventory.h"

int roll(int,int);
float rollf(float,float);
double rolld(double,double);

PlayerData::PlayerData(Context *context) : Object(context), energy_(0), currentattack_(PASpinAttack), inventoryscreen_(context)
{
}

Scene *PlayerData::GetCurrentScene()
{
	auto handler=GetSubsystem<GameStateHandler>();
	if(handler)
	{
		return handler->GetCurrentScene();
	}
	return nullptr;
}

void PlayerData::LoadItemModTable(const String &name)
{
	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
	if(file)
	{
		itemmodtable_.LoadJSON(file->GetRoot());
	}
}

void PlayerData::LoadItemModTiers(const String &name)
{
	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
	if(file)
	{
		itemmodtiers_.LoadJSON(file->GetRoot());
	}
}

void PlayerData::LoadBaseStats(const String &name)
{
	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
	if(file)
	{
		basestats_.Clear();
		basestats_.LoadJSON(file->GetRoot());
		levelmodifier_ = basestats_.AddMod("Level", StatModifier::FLAT, "1");
	}
}

void PlayerData::LoadItemClass(EquipmentSlots slot, const String name)
{
	if(slot>=EqNumEquipmentSlots) return;

	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
	if(file)
	{
		itemclasses_[slot].LoadJSON(file->GetRoot());
	}
}

void PlayerData::LoadSkillStats(const String &name)
{
	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
	skillstats_.clear();

	if(file)
	{
		// File must be an object
		const JSONValue &root=file->GetRoot();
		if(root.IsObject())
		{
			const JSONObject &obj=root.GetObject();
			for(auto i=obj.Begin(); i!=obj.End(); ++i)
			{
				String skillname=i->first_;
				const JSONValue &skillentry=i->second_;
				if(skillentry.IsObject())
				{
					const JSONObject &skill=skillentry.GetObject();
					StatSet stats;
					stats.LoadJSON(skill);
					skillstats_[std::string(skillname.CString())]=stats;
					Log::Write(LOG_INFO, String("Loaded skill ") + skillname);
				}
				else
				{
					Log::Write(LOG_ERROR, String("Skill entry ") + skillname + " is not an object.");
				}
			}
		}
		else
		{
			Log::Write(LOG_ERROR, "Skill stats file root must be an object.");
		}
	}
}

StatSet *PlayerData::GetSkillStatSet(const std::string &name)
{
	auto i=skillstats_.find(name);
	if(i==skillstats_.end()) return nullptr;
	return &(i->second);
}

StatSetCollection PlayerData::GetStatSetCollection(unsigned int slot, const std::string &skillname)
{
	// Build a collection.
	// Collect all item global stat sets, all buff stat sets, and all base stats
	// If slot<EqNumEquipmentSlots, collect local set from equipped item, if any
	// If skillname != "" collect the stats for the skill of the given name, if any

	StatSetCollection coll;
	coll.push_back(&basestats_);

	// Push global stats for all eq
	/*for(unsigned int g=0; g<EqNumEquipmentSlots; ++g)
	{
		coll.push_back(&equipmentglobalstats_[g]);
	}*/
	//coll.push_back(equipmentset_.GetGlobalStats());
	auto pi=GetSubsystem<PlayerInventory>();
	coll.push_back(pi->GetEquipment().GetGlobalStats());

	// Push local stats for requested slot
	/*if(slot<EqNumEquipmentSlots)
	{
		coll.push_back(&equipmentlocalstats_[slot]);
	}*/
	//auto imp=equipmentset_.GetLocalStats(slot);
	//if(imp) coll.push_back(imp);
	auto imp=pi->GetEquipment().GetLocalStats(slot);
	if(imp) coll.push_back(imp);

	if(skillname != "")
	{
		auto skill=GetSkillStatSet(skillname);
		if(skill) coll.push_back(skill);
	}

	return coll;
}

Node *PlayerData::GetPlayerNode()
{
	auto scene=GetCurrentScene();
	if(scene)
	{
		return scene->GetChild("Dude", true);
	}
	return nullptr;
}

/*
void PlayerData::EquipItem(const EquipmentItemDef &item, bool drop)
{
	if(item.slot_ < EqNumEquipmentSlots)
	{
		if(drop && equipment_[item.slot_].slot_!=EqNumEquipmentSlots)
		{
			// Drop currently equipped item
			auto nav=GetCurrentScene()->GetComponent<DynamicNavigationMesh>();
			auto pn=GetPlayerNode();
			if(pn)
			{
				DropItem(equipment_[item.slot_], pn->GetWorldPosition(), pn->GetWorldPosition());
			}
		}
		equipment_[item.slot_]=item;
		equipmentglobalstats_[item.slot_].Clear();
		equipmentlocalstats_[item.slot_].Clear();
		for(auto m : item.itemmods_)
		{
			auto mod = itemmodtable_.GetMod(m);
			if(mod)
			{
				if(mod->desig_ == IMImplicit || mod->desig_ == IMLocal)
				{
					// Add a local mod
					Log::Write(LOG_INFO, String("Added local mod ") + String(m) + ":" + String((int)mod->desig_));
					equipmentlocalstats_[item.slot_].Merge(mod->statset_);
				}
				else
				{
					Log::Write(LOG_INFO, String("Added global mod ") + String(m));
					equipmentglobalstats_[item.slot_].Merge(mod->statset_);
				}
			}
		}
	}

	auto ssc=GetVitalsStats();
	Log::Write(LOG_INFO, String("Life regen: ") + String(GetStatValue(ssc, "LifeRegen")));
}
*/

//void PlayerData::DropItem(const EquipmentItemDef &item, Vector3 dropperlocation, Vector3 location)
void PlayerData::DropItem(GeneralItem *item, Vector3 dropperlocation, Vector3 location)
{
	if(!GetCurrentScene()) return;

	//XMLFile *file=cache->GetResource<XMLFile>(item.dropobjectpath_);
	//Node *n=scene_->InstantiateXML(file->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	Node *n=GetCurrentScene()->CreateChild();

	auto nametag = n->CreateComponent<ItemNameTag>();
	if(nametag)
	{
		if(item->type_==GITEquipment)
		{
			nametag->SetItemName(item->def_.name_);
			nametag->SetItemColor(Color(1,1,1));
			switch(item->def_.rarity_)
			{
				case IRNormal: nametag->SetItemColor(Color(1,1,1)); break;
				case IRMagic: nametag->SetItemColor(Color(0.5,0.5,1)); break;
				case IRRare: nametag->SetItemColor(Color(1,1,0.5)); break;
				case IRUnique: nametag->SetItemColor(Color(1,0.75,0.25)); break;
			};
		}
	}
	else
	{
		Log::Write(LOG_INFO, "Name tag not created.");
	}

	auto dropitem = n->CreateComponent<DropItemContainer>();
	if(dropitem)
	{
		dropitem->SetItem(item);
	}
	else
	{
		Log::Write(LOG_INFO, "Drop item not created.");
	}

	auto navmesh=GetCurrentScene()->GetComponent<DynamicNavigationMesh>();
	if(navmesh) location = navmesh->FindNearestPoint(location);

	n->SetWorldPosition(location);
	Log::Write(LOG_INFO, String("Dropped item at ") + String(location.x_) + ", " + String(location.z_));
}

void PlayerData::NewPlayer()
{
	// Reset the game to a new player state
	LoadItemModTable("Tables/Items/itemmods.json");
	LoadItemModTiers("Tables/Items/itemmodtiers.json");
	LoadBaseStats("Tables/Player/base.json");
	LoadSkillStats("Tables/Skills/skillstats.json");
	LoadItemClass(EqBlade, "Tables/Items/blades.json");
	LoadItemClass(EqTurret, "Tables/Items/lasers.json");
	LoadItemClass(EqShell, "Tables/Items/shells.json");

	inventoryscreen_.Setup();

	auto pi=GetSubsystem<PlayerInventory>();
	auto item=AddItem(EquipmentItemDef(EqBlade, IRNormal, "Starter Blade", "", "Objects/DungeonBot3000/Textures/testblade.png;0 0 63 95", {"StarterBladeImplicit"}));
	pi->AddItem(item, true);

	item=AddItem(EquipmentItemDef(EqTurret, IRNormal, "Starter Laser", "", "Objects/DungeonBot3000/Textures/testblade.png;0 0 63 63", {"StarterLaserImplicit"}));
	pi->AddItem(item, true);

	item=AddItem(EquipmentItemDef(EqShell, IRNormal, "Starter Shell", "", "Objects/DungeonBot3000/Textures/testblade.png;0 0 63 95", {"StarterShellGlobal"}));
	pi->AddItem(item, true);
}

StatSetCollection &PlayerData::GetVitalsStats()
{
	vitalsstats_.clear();
	vitalsstats_.push_back(&basestats_);
	auto pi=GetSubsystem<PlayerInventory>();
	if(pi)
	{
		auto &eq=pi->GetEquipment();
		vitalsstats_.push_back(eq.GetGlobalStats());
	}

	return vitalsstats_;
}

void PlayerData::ShowInventoryScreen(bool show)
{
	inventoryscreen_.SetVisible(show);
}

bool PlayerData::IsInventoryScreenVisible()
{
	return inventoryscreen_.IsVisible();
}

GeneralItem *PlayerData::AddItem(const EquipmentItemDef &def)
{
	SharedPtr<GeneralItem> itm(new GeneralItem(context_, def));
	globalitemlist_.push_back(itm);
	return itm;
}

void PlayerData::RemoveItem(GeneralItem *item)
{
	for(unsigned int c=0; c<globalitemlist_.size(); ++c)
	{
		if(globalitemlist_[c].Get()==item)
		{
			globalitemlist_[c].Reset();
			globalitemlist_[c]=globalitemlist_[globalitemlist_.size()-1];
			globalitemlist_.pop_back();
		}
	}
}

void PlayerData::SpawnPlayer(Scene *scene, Vector3 location)
{
	if(!scene)
	{
		Log::Write(LOG_ERROR, "Can not spawn player: no scene attached.");
		return;
	}

	auto cache=scene->GetSubsystem<ResourceCache>();

	XMLFile *file=cache->GetResource<XMLFile>("Objects/DungeonBot3000/object.xml");
	Node *n=scene->InstantiateXML(file->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	auto rb=n->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("LBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}

	rb=n->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("RBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}
	n->SetWorldPosition(location);
}

//bool PlayerData::GenerateRandomItem(EquipmentItemDef &item, EquipmentSlots slot, ItemRarity rarity, int level)
GeneralItem *PlayerData::GenerateRandomEquipmentItem(EquipmentSlots slot, ItemRarity rarity, int level)
{
	EquipmentItemDef item;

	if(rarity==IRUnique) rarity=IRRare;   // TODO: Implement uniques
	item.rarity_=rarity;
	item.slot_=slot;

	item.objectpath_="Objects/DungeonBot3000/Textures/testblade.png;0 0 63 95";

	auto cls=GetItemClass(slot);
	if(!cls) return nullptr;

	ItemClassEntry *entry=cls->Choose(level);
	if(!entry) return nullptr;

	item.name_=entry->name_;
	item.itemmods_.clear();
	for(auto i : entry->fixed_) item.itemmods_.push_back(i);

	if(rarity==IRNormal)
	{
		return AddItem(item);
	}

	unsigned int nummods=0;
	if(rarity==IRMagic) nummods=roll(1,2);
	else nummods=roll(3,6);

	// Clamp num mods to the number of possible choices in the item entry's random category.
	// Populate a vector with all of the possible choices and shuffle it, then draw N choices.
	nummods=std::min(nummods, (unsigned int)entry->random_.size());
	std::vector<unsigned int> choices;
	for(unsigned int c=0; c<entry->random_.size(); ++c) choices.push_back(c);
	std::random_shuffle(choices.begin(), choices.end());

	for(unsigned int c=0; c<nummods; ++c)
	{
		String tiergroup=entry->random_[choices[c]];
		String mod=itemmodtiers_.Choose(tiergroup, level);
		item.itemmods_.push_back(mod);
	}
	return AddItem(item);
}
