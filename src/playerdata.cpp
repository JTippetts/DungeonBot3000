#include "playerdata.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>

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
