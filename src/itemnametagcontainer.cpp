#include "itemnametagcontainer.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>

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

	Log::Write(LOG_INFO, "Check mouse");
	auto element = ui->GetElementAt(mousepos);
	if(element)
	{
		Log::Write(LOG_INFO, "Checking hover");
		while(element->GetParent() != ui->GetRoot()) element=element->GetParent();
		auto nametag=dynamic_cast<ItemNameTag *>(element->GetVar(StringHash("ItemNameTag")).GetPtr());
		if(nametag)
		{
			Log::Write(LOG_INFO, "Found nametag");
		}
		return nametag;
	}
	return nullptr;
}

