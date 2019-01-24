#include "playerdata.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/IO/Log.h>

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

	// Find eq, pass for now

	if(skillname != "")
	{
		auto skill=GetSkillStatSet(skillname);
		if(skill) coll.push_back(skill);
	}

	return coll;
}
