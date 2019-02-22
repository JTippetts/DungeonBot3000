#include "itemnametagcontainer.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/Log.h>

#include "itemnametag.h"
#include "dropitem.h"
#include "thirdpersoncamera.h"
#include "../itemmods.h"
#include "../playerdata.h"
#include "../gamestatehandler.h"

#include <algorithm>

Intersection RectIntersect(IntRect &r1, IntRect r2)
{
	if(r2.right_ < r1.left_ || r2.left_ > r1.right_ || r2.bottom_ < r1.top_ || r2.top_ > r1.bottom_) return OUTSIDE;
	else if (r2.left_ < r1.left_ || r2.right_ > r1.right_ || r2.top_ < r1.top_ || r2.bottom_ > r1.bottom_) return INTERSECTS;
	else return INSIDE;
}

String WriteVector3(const Vector3 &v)
{
	return String("(")+String(v.x_)+","+String(v.y_)+","+String(v.z_)+")";
}

String WriteVector2(const Vector2 &v)
{
	return String("(")+String(v.x_)+","+String(v.y_)+")";
}


void ItemNameTagContainer::RegisterObject(Context *context)
{
	context->RegisterFactory<ItemNameTagContainer>();
}

ItemNameTagContainer::ItemNameTagContainer(Context *context) : LogicComponent(context), ready_(false)
{
}

void ItemNameTagContainer::AddNameTag(ItemNameTag *tag)
{
	tags_.push_back(WeakPtr<ItemNameTag>(tag));
}

void ItemNameTagContainer::RemoveNameTag(ItemNameTag *tag)
{
	//tags_.remove(tag);
	for(unsigned int i=0; i<tags_.size(); ++i)
	{
		if(tags_[i]==tag)
		{
			tags_[i]=tags_[tags_.size()-1];
			tags_.pop_back();
			return;
		}
	}
}

void ItemNameTagContainer::DelayedStart()
{

}

bool Collides(std::vector<ItemNameTag *> &fixed, ItemNameTag *tag, int &move)
{
	auto myelem=tag->GetElement();
	IntRect myrect=myelem->GetCombinedScreenRect();

	for(auto i : fixed)
	{
		auto elem=i->GetElement();

		IntRect rect=elem->GetCombinedScreenRect();
		auto intersect=RectIntersect(myrect, rect);
		if(intersect==INTERSECTS || intersect==INSIDE)
		{
			move=(rect.bottom_ - myrect.top_)+1;
			return true;
		}
	}
	return false;
}

void ItemNameTagContainer::Populate()
{
	std::vector<ItemNameTag *> visible;
	auto ui=GetSubsystem<UI>();
	auto layer=ui->GetRoot()->GetChild("ItemTagLayer", true);
	auto cam = node_->GetScene()->GetChild("Camera");//->GetComponent<ThirdPersonCamera>();
	//auto frustum = cam->GetFrustum();
	auto campos=cam->GetWorldPosition();

	std::vector<ItemNameTag *> fixed;

	if(layer)
	{
		layer->RemoveAllChildren();

		// Find all visible items and store in visible
		for(auto i=tags_.begin(); i!=tags_.end(); ++i)
		{
			// Get position and check visibility
			while((*i).Expired()) i=tags_.erase(i);
			if(i!=tags_.end())
			{
				auto pos = (*i)->GetNode()->GetWorldPosition();
				if((pos-campos).Length() < 100.0f)
				//if(frustum.IsInside(pos)==INSIDE)
				{
					visible.push_back((*i).Get());
					(*i)->SetScreenLocation();
					layer->AddChild((*i)->GetElement());
				}
			}
		}

		// Sort visible based on the x-z coordinate of the item drop
		std::sort(visible.begin(), visible.end(), [](ItemNameTag *t1, ItemNameTag *t2)->bool
			{
				// Sort based on x-z of item location
				auto p1 = t1->GetNode()->GetWorldPosition();
				auto p2 = t2->GetNode()->GetWorldPosition();
				return p1.x_ - p1.z_ < p2.x_ - p2.z_;
			}
		);

		//
		int move;
		for(auto i : visible)
		{
			while(Collides(fixed, i, move))
			{
				auto element=i->GetElement();
				auto pos = element->GetPosition();
				pos.y_ += move+1;
				element->SetPosition(pos);
			}
			fixed.push_back(i);
		}
	}
}

void ItemNameTagContainer::Update(float dt)
{
	Populate();
	DoItemHover();
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
		//while(element->GetParent() != ui->GetRoot()->GetChild("ItemTagLayer", true)) element=element->GetParent();
		if(element->GetParent() != ui->GetRoot()->GetChild("ItemTagLayer", true)) return nullptr;
		auto nametag=dynamic_cast<ItemNameTag *>(element->GetVar(StringHash("ItemNameTag")).GetPtr());
		return nametag;
	}
	return nullptr;
}

void ItemNameTagContainer::DoItemHover()
{
	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();
	auto pd=GetSubsystem<PlayerData>();
	auto input=GetSubsystem<Input>();

	auto &is=pd->GetInventoryScreen();

	// Don't do hover if over a HUD element
	IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=ui->GetCursorPosition();

	auto hudlayer=ui->GetRoot()->GetChild("HUDLayer", true);
	if(hudlayer)
	{
		unsigned int numchildren=hudlayer->GetNumChildren();
		for(unsigned int c=0; c<numchildren; ++c)
		{
			auto child=hudlayer->GetChild(c);
			auto rect=child->GetCombinedScreenRect();
			if(child->IsVisible() && rect.IsInside(mousepos)==INSIDE) return;
		}
	}

	ItemNameTag *tag=GetHoveredTag();
	if(!tag)
	{
		if(itemdesc_) itemdesc_->SetVisible(false);
		if(equippeddesc_) equippeddesc_->SetVisible(false);
		return;
	}

	auto nd = tag->GetNode();
	auto drop = nd->GetComponent<DropItemContainer>();

	if(!drop) return;

	WeakPtr<GeneralItem> item(drop->GetItem());
	if(!item || item.Expired()) return;

	auto &modtable=pd->GetItemModTable();

	if(item->type_==GITEquipment)
	{
		auto &def=item->def_;
		if(!itemdesc_)
		{
			itemdesc_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/ItemDescriptionBox.xml"));
			ui->GetRoot()->GetChild("AboveHUDLayer",true)->AddChild(itemdesc_);
		}

		if(!equippeddesc_)
		{
			equippeddesc_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/ItemDescriptionBox.xml"));
			ui->GetRoot()->GetChild("AboveHUDLayer", true)->AddChild(equippeddesc_);
			Text *title=dynamic_cast<Text *>(equippeddesc_->GetChild("Title", true));
			if(title) title->SetText("Equipped Item");
		}

		BuildItemDescription(itemdesc_, item, "Item on ground");



		// Equipped item
		// TODO
		/*EquipmentItemDef *eqdef=pd->GetEquipmentSlot(def.slot_);
		if(eqdef)
		{
			modlist=equippeddesc_->GetChild("ModList", true);
			modlist->RemoveAllChildren();
			AddItem(eqslot,modlist);
			for(auto md : eqdef->itemmods_)
			{
				auto entry = modtable.GetMod(md);
				if(entry)
				{
					AddItem(entry->desc_,modlist);
				}
			}
			equippeddesc_->SetPosition(IntVector2(0, graphics->GetHeight()/2 - equippeddesc_->GetHeight()/2));
			equippeddesc_->SetVisible(true);
		}*/
	}
}

void ItemNameTagContainer::Stop()
{
	if(itemdesc_)
	{
		itemdesc_->Remove();
	}
	if(equippeddesc_)
	{
		equippeddesc_->Remove();
	}
}
