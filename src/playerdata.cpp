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

#include "Components/dropitem.h"
#include "Components/itemnametag.h"

PlayerData::PlayerData(Context *context) : Object(context)
{
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
		basestats_.LoadJSON(file->GetRoot());
	}

	levelmodifier_ = basestats_.AddMod("Level", StatModifier::FLAT, "10");
}

void PlayerData::LoadSkillStats(const String &name)
{
	ResourceCache *cache=context_->GetSubsystem<ResourceCache>();
	JSONFile *file = cache->GetResource<JSONFile>(name);
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

StatSetCollection PlayerData::GetStatSetCollection(EquipmentSlots slot, const std::string &skillname)
{
	// Build a collection.
	// Collect all item global stat sets, all buff stat sets, and all base stats
	// If slot<EqNumEquipmentSlots, collect local set from equipped item, if any
	// If skillname != "" collect the stats for the skill of the given name, if any

	StatSetCollection coll;
	coll.push_back(&basestats_);

	// Push global stats for all eq
	for(unsigned int g=0; g<EqNumEquipmentSlots; ++g)
	{
		coll.push_back(&equipmentglobalstats_[g]);
	}

	// Push local stats for requested slot
	if(slot<EqNumEquipmentSlots)
	{
		coll.push_back(&equipmentlocalstats_[slot]);
	}

	if(skillname != "")
	{
		auto skill=GetSkillStatSet(skillname);
		if(skill) coll.push_back(skill);
	}

	return coll;
}

void PlayerData::EquipItem(const EquipmentItemDef &item, bool drop)
{
	if(item.slot_ < EqNumEquipmentSlots)
	{
		if(drop && equipment_[item.slot_].slot_!=EqNumEquipmentSlots)
		{
			// Drop currently equipped item
			DropItem(equipment_[item.slot_], Vector3(0,0,0), Vector3(0,0,0));
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
					equipmentlocalstats_[item.slot_].Merge(mod->statset_);
				}
				else
				{
					equipmentglobalstats_[item.slot_].Merge(mod->statset_);
				}
			}
		}
	}
}

void PlayerData::DropItem(const EquipmentItemDef &item, Vector3 dropperlocation, Vector3 location)
{
	if(!currentscene_) return;

	//XMLFile *file=cache->GetResource<XMLFile>(item.dropobjectpath_);
	//Node *n=scene_->InstantiateXML(file->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	Node *n=currentscene_->CreateChild();

	auto nametag = n->CreateComponent<ItemNameTag>();
	if(nametag)
	{
		nametag->SetItemName(item.name_);
		nametag->SetItemColor(Color(1,1,1));
		switch(item.rarity_)
		{
			case IRNormal: nametag->SetItemColor(Color(1,1,1)); break;
			case IRMagic: nametag->SetItemColor(Color(0.25,0.25,1)); break;
			case IRRare: nametag->SetItemColor(Color(1,1,0)); break;
			case IRUnique: nametag->SetItemColor(Color(1,0.75,0)); break;
		};
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

	auto navmesh=currentscene_->GetComponent<DynamicNavigationMesh>();
	if(navmesh) location = navmesh->FindNearestPoint(location);

	n->SetWorldPosition(location);
}

