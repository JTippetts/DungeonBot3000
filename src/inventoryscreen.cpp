#include "inventoryscreen.h"
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Input/Input.h>

#include "playerdata.h"
#include "playerinventory.h"

float rollf(float,float);

InventoryScreen::InventoryScreen(Context *context) : Object(context), bagelement_(nullptr), equipelement_(nullptr), bagwidth_(0), bagheight_(0), handitem_(nullptr)
{
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(InventoryScreen, HandleUpdate));
}

void InventoryScreen::Setup()
{
	if(element_)
	{
		element_->Remove();
		element_.Reset();
	}
	auto ui=GetSubsystem<UI>();
	auto graphics=GetSubsystem<Graphics>();
	auto cache=GetSubsystem<ResourceCache>();

	element_=SharedPtr<UIElement>(new UIElement(context_));
	element_->SetSize(IntVector2(320, 480));

	auto slotelement=element_->CreateChild<UIElement>("SlotElement");
	slotelement->SetSize(IntVector2(256, 256));
	slotelement->SetPosition(IntVector2(32,0));

	auto pi=GetSubsystem<PlayerInventory>();
	if(!pi) return;

	auto &bag=pi->GetBag();
	auto &equip=pi->GetEquipment();

	BuildBag(10,5);
	bagwidth_=10;
	bagheight_=5;


	// Build slots
	BuildSlot(StringHash("MainHand"),0,0,2,3,"Blade");
	BuildSlot(StringHash("Turret"),96,0,2,2,"Turret");
	BuildSlot(StringHash("Offhand"),192,0,2,3,"Blade;Shield");
	BuildSlot(StringHash("DriveSystem"),0,128,2,2,"DriveSystem");
	BuildSlot(StringHash("Shell"),96,96,2,3,"Shell");
	BuildSlot(StringHash("GeneratorSystem"),192,128,2,2,"GeneratorSystem");
	for(unsigned int c=0; c<8; ++c)
	{
		BuildSlot(StringHash(String("Processor")+String(c)),c*32,226,1,1,"Processor");
	}

	element_->SetPosition(IntVector2(graphics->GetWidth()/2, graphics->GetHeight()/4));
	element_->SetVisible(false);
	ui->GetRoot()->GetChild("HUDLayer", true)->AddChild(element_);

	hoveredelement_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/ItemDescriptionBox.xml"));
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(hoveredelement_);
	hoveredelement_->SetVisible(false);
}

void InventoryScreen::BuildBag(unsigned int width, unsigned int height)
{
	auto cache=GetSubsystem<ResourceCache>();
	auto pi=GetSubsystem<PlayerInventory>();
	if(!pi) return;

	pi->GetBag().Setup(width, height);

	if(element_)
	{
		bagelement_=element_->CreateChild<UIElement>("Bag");
		bagelement_->SetSize(width*32, height*32);
		bagelement_->SetPosition(IntVector2(0,288));

		bagimageelement_=element_->CreateChild<UIElement>("BagImages");
		bagimageelement_->SetSize(width*32, height*32);
		bagimageelement_->SetPosition(IntVector2(0,288));

		for(unsigned int y=0; y<height; ++y)
		{
			for(unsigned int x=0; x<width; ++x)
			{
				auto slot=bagelement_->CreateChild<BorderImage>("Background");
				slot->SetTexture(cache->GetResource<Texture2D>("UI/buttons.png"));
				slot->SetImageRect(IntRect(24,154,25,155));
				slot->SetColor(Color(0.5,0.5,0.5));
				slot->SetSize(IntVector2(32,32));
				slot->SetPosition(IntVector2(x*32, y*32));

				auto frame=slot->CreateChild<BorderImage>("Frame");
				frame->SetTexture(cache->GetResource<Texture2D>("UI/buttons.png"));
				frame->SetImageRect(IntRect(192,384,223,415));
				frame->SetBorder(IntRect(8,8,8,8));
				frame->SetSize(32,32);

				bagslots_.push_back(slot);

			}
		}
	}

}

void InventoryScreen::BuildSlot(StringHash name, unsigned int x, unsigned int y, unsigned int width, unsigned int height, const String &allowedtypes)
{
	if(!element_) return;

	auto pi=GetSubsystem<PlayerInventory>();
	if(!pi) return;
	auto cache=GetSubsystem<ResourceCache>();

	equipelement_=element_->GetChild("SlotElement", true);
	if(!equipelement_) return;

	auto sl=equipelement_->CreateChild<BorderImage>("Background");
	sl->SetImageRect(IntRect(24,154,25,155));
	sl->SetColor(Color(0.5,0.5,0.5));
	sl->SetSize(IntVector2(32*width, 32*height));
	sl->SetTexture(cache->GetResource<Texture2D>("UI/buttons.png"));

	auto frame=sl->CreateChild<BorderImage>("Frame");
	frame->SetTexture(cache->GetResource<Texture2D>("UI/buttons.png"));
	frame->SetImageRect(IntRect(192,384,223,415));
	frame->SetBorder(IntRect(8,8,8,8));
	frame->SetSize(32*width,32*height);
	sl->SetPosition(IntVector2(x,y));

	equipslots_[name]=sl;
	pi->AddEquipmentSlot(name, allowedtypes);
	return;
}

void InventoryScreen::SetVisible(bool vis)
{
	if(element_) element_->SetVisible(vis);
}

bool InventoryScreen::IsVisible()
{
	if(element_) return element_->IsVisible();
	return false;
}

void InventoryScreen::UpdateBagImages()
{
	if(!bagimageelement_) return;
	auto cache=GetSubsystem<ResourceCache>();
	auto pi=GetSubsystem<PlayerInventory>();
	auto &bag=pi->GetBag();
	bagimageelement_->RemoveAllChildren();
	// Update bag images
	auto &bagitems=bag.GetItems();
	for(unsigned int c=0; c<bagitems.size(); ++c)
	{
		auto item=bagitems[c];
		auto elem=bagimageelement_->CreateChild<BorderImage>();
		auto splits=item->inventoryimage_.Split(';');
		// String format: Texture name;Rectangle
		if(splits.Size()>=2)
		{
			elem->SetTexture(cache->GetResource<Texture2D>(splits[0]));
			IntRect rect=FromString<IntRect>(splits[1]);
			elem->SetImageRect(rect);
		}
		elem->SetSize(IntVector2(item->invsize_.x_*32, item->invsize_.y_*32));
		elem->SetPosition(IntVector2(item->invlocation_.x_*32, item->invlocation_.y_*32));
	}
}

void InventoryScreen::UpdateEquipmentImages()
{
	auto pi=GetSubsystem<PlayerInventory>();
	auto &equip=pi->GetEquipment();
	auto cache=GetSubsystem<ResourceCache>();

	for(auto i=equipslots_.Begin(); i!=equipslots_.End(); ++i)
	{
		StringHash name=(*i).first_;
		UIElement *eqelem=(*i).second_;
		if(eqelem)
		{
			auto oldelem=eqelem->GetChild("Image", true);
			if(oldelem) oldelem->Remove();

			unsigned int slot;
			if(pi->GetSlot(name, slot))
			{
				auto item=equip.GetItemAtSlot(slot);
				if(item)
				{
					auto elem=eqelem->CreateChild<BorderImage>("Image");
					if(elem)
					{
						auto splits=item->inventoryimage_.Split(';');
						// String format: Texture name;Rectangle
						if(splits.Size()>=2)
						{
							elem->SetTexture(cache->GetResource<Texture2D>(splits[0]));
							IntRect rect=FromString<IntRect>(splits[1]);
							elem->SetImageRect(rect);
						}
						elem->SetSize(IntVector2(item->invsize_.x_*32, item->invsize_.y_*32));
						elem->SetPosition(IntVector2(0,0));
					}
				}
			}
		}
	}
}

void InventoryScreen::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
	auto pi=GetSubsystem<PlayerInventory>();
	auto input=GetSubsystem<Input>();
	auto ui=GetSubsystem<UI>();

	if(pi->GetBag().IsDirty())
	{
		UpdateBagImages();
	}
	if(pi->GetEquipment().IsDirty())
	{
		UpdateEquipmentImages();
	}

	// Do hover
	if(hoveredelement_) hoveredelement_->SetVisible(false);
	if(IsVisible())
	{
		IntVector2 location;
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=ui->GetCursorPosition();
		unsigned int slot;
		UIElement *elem;


		if(GetHoveredBagLocation(location, mousepos))
		{
			auto item=pi->GetBag().FindItemInSlot(location);
			if(item)
			{
				if(hoveredelement_)
				{
					BuildItemDescription(hoveredelement_, item, "Item in Inventory");
					hoveredelement_->SetVisible(true);
					hoveredelement_->SetPosition(IntVector2(element_->GetPosition().x_ - hoveredelement_->GetWidth(), element_->GetPosition().y_));
				}
			}
		}
		else if(GetHoveredEquipmentSlot(slot, elem, mousepos))
		{
			auto item=pi->GetEquipment().GetItemAtSlot(slot);
			if(item)
			{
				if(hoveredelement_)
				{
					BuildItemDescription(hoveredelement_, item, "Equipped Item");
					hoveredelement_->SetVisible(true);
					hoveredelement_->SetPosition(IntVector2(element_->GetPosition().x_ - hoveredelement_->GetWidth(), element_->GetPosition().y_));
				}
			}
		}

		auto inhand=GetItemInHand();
		if(inhand)
		{
			if(handitem_)
			{
				handitem_->SetPosition(mousepos-IntVector2(-1,-1));
			}
		}

		// Check for lmb/rmb
		if(input->GetMouseButtonPress(MOUSEB_LEFT))
		{
			// If no item in hand, and over an item in bag, pick it up
			auto iteminhand=GetItemInHand();
			if(!iteminhand)
			{
				if(GetHoveredBagLocation(location, mousepos))
				{
					auto item=pi->GetBag().FindItemInSlot(location);
					if(item)
					{
						pi->GetBag().RemoveItem(item);
						PutItemInHand(item);
					}
				}
				else if(GetHoveredEquipmentSlot(slot, elem, mousepos)) // Hovering over a slot
				{
					auto item=pi->GetEquipment().GetItemAtSlot(slot);
					if(item)
					{
						pi->GetEquipment().RemoveItemFromSlot(slot);
						PutItemInHand(item);
					}
				}
			}
			else // Have item in hand
			{
				// HAve an item in hand, check to see if we can place it in inventory
				if(GetHoveredBagLocation(location, mousepos))
				{
					unsigned int coveredcount=pi->GetBag().CountCoveredItems(location, iteminhand->invsize_);
					if(coveredcount==0) // Not covering any items in inventory, check if can place it and place it
					{
						if(pi->GetBag().CanPlaceAtLocation(location, iteminhand, false))
						{
							// No covered items, and can place there so place it
							pi->GetBag().PlaceAtLocation(location, iteminhand);
							RemoveItemInHand();
						}
					}
					else if(coveredcount==1)
					{
						if(pi->GetBag().CanPlaceAtLocation(location, iteminhand, false))
						{
							// Covering 1 item, can place item there, so swap
							auto ci = pi->GetBag().GetCoveredItem(location, iteminhand->invsize_);
							if(ci)
							{
								auto &bag=pi->GetBag();
								bag.RemoveItem(ci);
								bag.PlaceAtLocation(location, iteminhand);
								RemoveItemInHand();
								PutItemInHand(ci);
							}
							else // If this happens, it's a bug in getcovereditem
							{
								Log::Write(LOG_INFO, "wut");
							}
						}
					}
					else
					{
						// Too many items, can't do anything
					}
				}
				else if(GetHoveredEquipmentSlot(slot, elem, mousepos)) // Item in hand, hovered over equipment slot
				{
					auto item=pi->GetEquipment().GetItemAtSlot(slot);
					if(item)   // There is an item there
					{
						if(pi->GetEquipment().CanAddItemToSlot(slot, iteminhand)) // Can add item in hand into slot
						{
							// Swap item in hand with item in equipment
							pi->GetEquipment().RemoveItemFromSlot(slot);
							pi->GetEquipment().AddItemToSlot(slot, iteminhand);
							RemoveItemInHand();
							PutItemInHand(item);
						}
						else  // Can not equip item in hand to that slot, do nothing
						{
						}
					}
					else // No item there
					{
						if(pi->GetEquipment().CanAddItemToSlot(slot, iteminhand)) // Can add the item there, so add it
						{
							RemoveItemInHand();
							pi->GetEquipment().AddItemToSlot(slot, iteminhand);
						}
						else // Can't add it to that slot, do nothing
						{
						}
					}
				}
				else // Have item in hand, drop it on ground
				{
					auto pd=GetSubsystem<PlayerData>();
					auto pn=pd->GetPlayerNode();
					RemoveItemInHand();
					pd->DropItem(iteminhand, pn->GetWorldPosition(), pn->GetWorldPosition()+Vector3(rollf(-5.0,5.0), 0, rollf(-5.0,5.0)));
				}
			}
		}
		else if(input->GetMouseButtonPress(MOUSEB_RIGHT))
		{
		}
	}
}

bool InventoryScreen::GetHoveredBagLocation(IntVector2 &location, const IntVector2 &mousepos)
{
	// Calculate which bag slot the mouse is over
	// Return false if not over any
	if(!bagelement_) return false;
	auto rect=bagelement_->GetCombinedScreenRect();
	if(rect.IsInside(mousepos)==OUTSIDE) return false;

	IntVector2 bagpos=mousepos - bagelement_->GetScreenPosition();
	location = bagpos / 32;
	return true;
}

bool InventoryScreen::GetHoveredEquipmentSlot(unsigned int &slot, UIElement * &element, const IntVector2 &mousepos)
{
	if(!equipelement_) return false;
	for(auto i=equipslots_.Begin(); i!=equipslots_.End(); ++i)
	{
		auto elem=(*i).second_;
		auto rect=elem->GetCombinedScreenRect();
		if(rect.IsInside(mousepos))
		{
			auto pi=GetSubsystem<PlayerInventory>();
			if(pi->GetSlot((*i).first_, slot))
			{
				element=elem;
				return true;
			}
		}
	}
	return false;
}

void InventoryScreen::ResetBagColors()
{
	for(unsigned int x=0; x<bagwidth_; ++x)
	{
		for(unsigned int y=0; y<bagheight_; ++y)
		{
			unsigned int index=y*bagwidth_+x;
			if(index<bagslots_.size())
			{
				auto slot=bagslots_[index];
				slot->GetChild("Background",true)->SetColor(Color(0.5,0.5,0.5));
			}
		}
	}
}

void InventoryScreen::PutItemInHand(GeneralItem *item)
{
	if(!item) return;
	if(handitem_)
	{
		handitem_->Remove();
		handitem_.Reset();
	}

	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto hudlayer=ui->GetRoot()->GetChild("HUDLayer", true);
	auto &invsize=item->invsize_;

	iteminhand_=WeakPtr<GeneralItem>(item);
	SharedPtr<BorderImage> hi(hudlayer->CreateChild<BorderImage>());
	hi->SetTexture(cache->GetResource<Texture2D>("UI/buttons.png"));
	hi->SetImageRect(IntRect(26,152,27,153));
	hi->SetColor(Color(0.5,0.5,0.5));
	hi->SetOpacity(0.5);
	hi->SetSize(invsize*32);

	auto chld=hi->CreateChild<BorderImage>();
	auto splits=item->inventoryimage_.Split(';');
	if(splits.Size()>=2)
	{
		chld->SetTexture(cache->GetResource<Texture2D>(splits[0]));
		IntRect rect=FromString<IntRect>(splits[1]);
		chld->SetImageRect(rect);
		chld->SetSize(invsize*32);
	}
	handitem_ = SharedPtr<UIElement>(hi);
}

void InventoryScreen::RemoveItemInHand()
{
	iteminhand_=WeakPtr<GeneralItem>(nullptr);

	if(handitem_)
	{
		handitem_->Remove();
		handitem_.Reset();
	}
}

GeneralItem * InventoryScreen::GetItemInHand()
{
	if(!iteminhand_ || iteminhand_.Expired()) return nullptr;
	return iteminhand_;
}
