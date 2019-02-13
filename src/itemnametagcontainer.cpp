#include "itemnametagcontainer.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/Font.h>

#include "Components/itemnametag.h"
#include "Components/dropitem.h"
#include "itemmods.h"
#include "playerdata.h"

ItemNameTagContainer::ItemNameTagContainer(Context *context) : Object(context)
{
}

void ItemNameTagContainer::AddNameTag(ItemNameTag *tag)
{
}

void ItemNameTagContainer::RemoveNameTag(ItemNameTag *tag)
{

}

ItemNameTag *ItemNameTagContainer::GetHoveredTag()
{
	auto ui=GetSubsystem<UI>();
	auto input=GetSubsystem<Input>();

	IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=ui->GetCursorPosition();

	auto element = ui->GetElementAt(mousepos);
	if(element)
	{
		while(element->GetParent() != ui->GetRoot()) element=element->GetParent();
		auto nametag=dynamic_cast<ItemNameTag *>(element->GetVar(StringHash("ItemNameTag")).GetPtr());
		return nametag;
	}
	return nullptr;
}

void ItemNameTagContainer::DoItemHover()
{
	ItemNameTag *tag=GetHoveredTag();
	if(!tag)
	{
		if(itemdesc_) itemdesc_->SetVisible(false);
		return;
	}

	auto nd = tag->GetNode();
	auto drop = nd->GetComponent<DropItemContainer>();

	if(!drop) return;

	auto def=drop->GetItem();
	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();
	auto pd=GetSubsystem<PlayerData>();
	auto &modtable=pd->GetItemModTable();

	if(!itemdesc_)
	{
		itemdesc_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/ItemDescriptionBox.xml"));
		ui->GetRoot()->AddChild(itemdesc_);
	}

	String eqslot;
	switch(def.slot_)
	{
		case EqBlade: eqslot="Blade"; break;
		case EqTurret: eqslot="Turret"; break;
		case EqAntenna: eqslot="Antenna"; break;
		case EqProcessor: eqslot="Processor"; break;
		case EqDriveSystem: eqslot="Drive System"; break;
		case EqGeneratorSystem: eqslot="Generator System"; break;
		default: eqslot="Unknown"; break;
	};

	auto modlist=itemdesc_->GetChild("ModList", true);
	modlist->RemoveAllChildren();

	auto AddItem=[modlist, cache](const String &item)
	{
		auto txt = modlist->CreateChild<Text>();
		txt->SetTextAlignment(HA_CENTER);
		txt->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"));
		txt->SetFontSize(14);
		txt->SetText(item);
	};

	AddItem(eqslot);

	for(auto md : def.itemmods_)
	{
		auto entry = modtable.GetMod(md);
		if(entry)
		{
			AddItem(entry->desc_);
		}
	}

	itemdesc_->SetPosition(IntVector2(graphics->GetWidth()-itemdesc_->GetWidth(), graphics->GetHeight()/2 - itemdesc_->GetHeight()/2));
	itemdesc_->SetVisible(true);
}
