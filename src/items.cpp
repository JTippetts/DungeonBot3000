#include "items.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/IO/Log.h>
#include "playerdata.h"


void BuildItemDescription(UIElement *desc, GeneralItem *item, const String &title)
{
	if(!desc || !item) return;
	auto cache=desc->GetSubsystem<ResourceCache>();
	auto graphics=desc->GetSubsystem<Graphics>();
	auto pd=desc->GetSubsystem<PlayerData>();
	auto &modtable=pd->GetItemModTable();

	Text *ttl=dynamic_cast<Text *>(desc->GetChild("Title", true));
	if(ttl) ttl->SetText(title);
	if(item->type_ == GITEquipment)
	{
		auto &def=item->def_;
		String eqslot;
		switch(def.slot_)
		{
			case EqBlade: eqslot="Blade"; break;
			case EqTurret: eqslot="Turret"; break;
			case EqShell: eqslot="Shell"; break;
			case EqProcessor: eqslot="Processor"; break;
			case EqDriveSystem: eqslot="Drive System"; break;
			case EqShield: eqslot="Shield"; break;
			case EqGeneratorSystem: eqslot="Generator System"; break;
			default: eqslot="Unknown"; break;
		};

		auto modlist=desc->GetChild("ModList", true);
		if(modlist) modlist->RemoveAllChildren();

		auto AddItem=[cache](const String &item, UIElement *modlist)
		{
			if(!modlist) return;
			auto txt = modlist->CreateChild<Text>();
			txt->SetTextAlignment(HA_CENTER);
			txt->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"));
			txt->SetFontSize(14);
			txt->SetText(item);
		};

		AddItem(eqslot,modlist);

		for(auto md : def.itemmods_)
		{
			auto entry = modtable.GetMod(md);
			if(entry)
			{
				AddItem(entry->desc_,modlist);
			}
		}

		desc->SetPosition(IntVector2(graphics->GetWidth()-desc->GetWidth(), graphics->GetHeight()/2 - desc->GetHeight()/2));
		desc->SetVisible(true);
	}
}
