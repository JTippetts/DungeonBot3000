#include "itemmods.h"
#include "jsonutil.h"
#include <Urho3D/IO/Log.h>
#include "weightedtable.h"

double rolld(double low, double high);

ItemModTable::ItemModTable()
{
}

void ItemModTable::LoadJSON(const JSONValue &json)
{
	StringHasherType StringHasher;
	if(!json.IsObject())
	{
		Log::Write(LOG_ERROR, "JSON file not an object");
		return; // Needs to be an object
	}

	mods_.clear();

	const JSONObject &obj=json.GetObject();
	// Object with name: descriptor
	for(auto i=obj.Begin(); i!=obj.End(); ++i)
	{
		std::string name=i->first_.CString();
		const JSONValue &val=i->second_;
		if(val.IsArray())
		{
			// Array with [description, statset]
			const JSONArray &arr=val.GetArray();
			if(arr.Size()>=3)
			{
				String designation=arr[0].GetString();
				ItemModDesignation desig=IMImplicit;
				if(designation=="Local") desig=IMLocal;
				else if(designation=="Global") desig=IMGlobal;

				Log::Write(LOG_INFO, String("Item mod designation: ") + designation + ":" + String(int(desig)));

				String desc=arr[1].GetString();
				const JSONValue &ss=arr[2];
				if(ss.IsObject())
				{
					const JSONObject &sso=ss.GetObject();
					StatSet statset;
					statset.LoadJSON(sso);
					mods_[name]=ItemModEntry(desc,statset,desig);
					Log::Write(LOG_INFO, String("Loaded item mod ") + String(name.c_str()) + ": " + desc);
				}
				else
				{
				}
			}
		}
		else
		{
			Log::Write(LOG_ERROR, "Error");
		}
	}
}

ItemModEntry *ItemModTable::GetMod(const std::string &name)
{
	auto seti = mods_.find(name);
	if(seti != mods_.end())
	{
		return &seti->second;
	}
	return nullptr;
}

ItemModEntry *ItemModTable::GetMod(const String &name)
{
	std::string nm(name.CString());
	auto seti = mods_.find(nm);
	if(seti != mods_.end())
	{
		return &seti->second;
	}
	return nullptr;
}

ItemModTierTable::ItemModTierTable()
{
}

void ItemModTierTable::LoadJSON(const JSONValue &json)
{
	// Format
	// Object {
	// Weighting: Number
	// Tables: Array
	//    Object {
	//		Level: Number (minimum level it can appear)
	//      Mods: Array
	//			String (name of mod in master mod list)
	table_.clear();

	if(json.IsObject())
	{
		const JSONObject &obj=json.GetObject();
		weighting_=GetDoubleFromJSONObject("Weighting", obj);
		const JSONValue *tables=obj[String("Tables")];
		if(tables)
		{
			if(tables->IsArray())
			{
				const JSONArray &tablesarray=tables->GetArray();
				for(unsigned int entry=0; entry<tablesarray.Size(); ++entry)
				{
					const JSONValue &tableentry=tablesarray[entry];
					if(tableentry.IsObject())
					{
						const JSONObject &entryobject=tableentry.GetObject();
						int minlevel=(int)GetDoubleFromJSONObject(String("Level"),entryobject);
						const JSONValue *mods=entryobject[String("Mods")];
						if(mods)
						{
							if(mods->IsArray())
							{
								const JSONArray &modlist=mods->GetArray();
								ItemModTierSet tierset;
								tierset.minlevel_=minlevel;
								for(unsigned int ml=0; ml<modlist.Size(); ++ml)
								{
									tierset.modlist_.push_back(modlist[ml].GetString());
								}
								table_.push_back(tierset);
							}
							else
							{
								Log::Write(LOG_ERROR, "ItemModifierTiersTable Tables entry Mod list must be an array");
							}
						}
						else
						{
							Log::Write(LOG_ERROR, "ItemModifierTiersTable Tables entry Mod list does not exist.");
						}
					}
					else
					{
						Log::Write(LOG_ERROR, "ItemModifierTiersTable Tables entry must be an object");
					}
				}
			}
			else
			{
				Log::Write(LOG_ERROR, "ItemModTiersTable Tables must be an array");
			}
		}
		else
		{
			Log::Write(LOG_ERROR, "ItemModTiersTable Tables entry does not exist.");
		}
	}
	else
	{
		Log::Write(LOG_ERROR, "ItemModTiersTable entry must be an object.");
	}
}

String ItemModTierTable::Choose(int level)
{
	// Iterate until the highest minlevel is found that is still less than or equal to level
	int highest=-1;
	for(unsigned int i=0; i<table_.size(); ++i)
	{
		if(table_[i].minlevel_ <= level) highest=i;
	}

	if(highest==-1)
	{
		// No mod table found for that level
		return String("");
	}

	int numentries=table_[highest].modlist_.size();
/*
	std::vector<double> weights;
	double total=0.0;

	for(int i=0; i<numentries; ++i)
	{
		double wt=std::pow(weighting_, (double)i);
		total += wt;
		weights.push_back(total);
	}

	for(int i=0; i<numentries; ++i) weights[i] /= total;
	//for(int i=1; i<numentries; ++i) weights[i]=weights[i]+weights[i-1];

	double rl=rolld(0.0, 1.0);

	for(int i=0; i<numentries; ++i)
	{
		if(rl < weights[i])
		{
			return table_[highest].modlist_[i];
		}
	}*/
	WeightedTable tbl(weighting_, numentries);
	int choice=tbl.Choose();
	if(choice < table_[highest].modlist_.size()) return table_[highest].modlist_[choice];

	// No mod found
	return String("");
}


ItemModTiers::ItemModTiers()
{
}

void ItemModTiers::LoadJSON(const JSONValue &json)
{
	if(json.IsObject())
	{
		const JSONObject &obj=json.GetObject();
		for(auto i=obj.Begin(); i!=obj.End(); ++i)
		{
			std::string name=i->first_.CString();
			Log::Write(LOG_INFO, String("Loading tier group ") + i->first_);
			const JSONValue &val=i->second_;
			ItemModTierTable md;
			md.LoadJSON(val);
			map_[name]=md;
		}
	}
	else
	{
		Log::Write(LOG_ERROR, "ItemModTiers file must be an object.");
	}
}

String ItemModTiers::Choose(const String &which, int level)
{
	return Choose(std::string(which.CString()), level);
}

String ItemModTiers::Choose(const std::string &which, int level)
{
	auto i = map_.find(which);
	if(i != map_.end())
	{
		return i->second.Choose(level);
	}
	else
	{
		Log::Write(LOG_ERROR, String("Attempt to request mod tier group ") + String(which.c_str()) + " which does not exist.");
		return String("");
	}
}

void ItemClass::LoadJSON(const JSONValue &json)
{
	entries_.clear();
	if(json.IsArray())
	{
		const JSONArray &arr=json.GetArray();
		for(unsigned int cl=0; cl<arr.Size(); ++cl)
		{
			const JSONValue &tableentry=arr[cl];
			if(tableentry.IsObject())
			{
				const JSONObject &entryobject=tableentry.GetObject();
				ItemClassEntry entry;
				entry.name_ = GetStringFromJSONObject("Name", entryobject);
				entry.minlevel_ = (int)GetDoubleFromJSONObject("MinLevel", entryobject);
				entry.weight_ = GetDoubleFromJSONObject("Weight", entryobject);

				const JSONValue *fixed=entryobject[String("Fixed")];
				if(fixed)
				{
					if(fixed->IsArray())
					{
						const JSONArray fixarr=fixed->GetArray();
						for(unsigned int f=0; f<fixarr.Size(); ++f)
						{
							entry.fixed_.push_back(fixarr[f].GetString());
						}
					}
					else
					{
						Log::Write(LOG_ERROR, "Fixed modifiers must be an array.");
					}
				}

				const JSONValue *random=entryobject[String("Random")];
				if(random)
				{
					if(random->IsArray())
					{
						const JSONArray fixarr=random->GetArray();
						for(unsigned int f=0; f<fixarr.Size(); ++f)
						{
							entry.random_.push_back(fixarr[f].GetString());
						}
					}
					else
					{
						Log::Write(LOG_ERROR, "Random modifiers must be an array.");
					}
				}

				entries_.push_back(entry);
			}
			else
			{
				Log::Write(LOG_ERROR, "ItemClass entry must be object.");
			}
		}
	}
	else
	{
		Log::Write(LOG_ERROR, "ItemClass file must be an array.");
	}
}

ItemClassEntry *ItemClass::Choose(int level)
{
	// Iterate and add options into a list
	std::vector<ItemClassEntry *> options;
	std::vector<double> weights;
	double totalweight=0.0;

	for(unsigned int c=0; c<entries_.size(); ++c)
	{
		ItemClassEntry *e = &entries_[c];
		if(e->minlevel_<=level)
		{
			options.push_back(e);
			totalweight+=e->weight_;
			weights.push_back(totalweight);

		}
	}

	for(unsigned int c=0; c<weights.size(); ++c)
	{
		weights[c] /= totalweight;
	}

	double rl=rolld(0.0, 1.0);
	for(unsigned int c=0; c<weights.size(); ++c)
	{
		if(rl < weights[c]) return options[c];
	}

	return nullptr;
}
