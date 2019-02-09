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

PlayerData::PlayerData(Context *context) : Object(context)
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

Node *PlayerData::GetPlayerNode()
{
	auto scene=GetCurrentScene();
	if(scene)
	{
		return scene->GetChild("Dude", true);
	}
	return nullptr;
}

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

void PlayerData::DropItem(const EquipmentItemDef &item, Vector3 dropperlocation, Vector3 location)
{
	if(!GetCurrentScene()) return;

	//XMLFile *file=cache->GetResource<XMLFile>(item.dropobjectpath_);
	//Node *n=scene_->InstantiateXML(file->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	Node *n=GetCurrentScene()->CreateChild();

	auto nametag = n->CreateComponent<ItemNameTag>();
	if(nametag)
	{
		nametag->SetItemName(item.name_);
		nametag->SetItemColor(Color(1,1,1));
		switch(item.rarity_)
		{
			case IRNormal: nametag->SetItemColor(Color(1,1,1)); break;
			case IRMagic: nametag->SetItemColor(Color(0.5,0.5,1)); break;
			case IRRare: nametag->SetItemColor(Color(1,1,0.5)); break;
			case IRUnique: nametag->SetItemColor(Color(1,0.75,0.25)); break;
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

	auto navmesh=GetCurrentScene()->GetComponent<DynamicNavigationMesh>();
	if(navmesh) location = navmesh->FindNearestPoint(location);

	n->SetWorldPosition(location);
}

void PlayerData::NewPlayer()
{
	// Reset the game to a new player state
	LoadItemModTable("Tables/Items/itemmods.json");
	LoadItemModTiers("Tables/Items/itemmodtiers.json");
	LoadBaseStats("Tables/Player/base.json");
	LoadSkillStats("Tables/Skills/skillstats.json");

	EquipItem(EquipmentItemDef(EqBlade, IRNormal, "Starter Blade", "", "", {"StarterBladeImplicit"}), false);
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
