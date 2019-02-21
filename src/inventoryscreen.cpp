#include "inventoryscreen.h"
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>

#include "playerdata.h"

InventoryScreen::InventoryScreen(Context *context) : Object(context), bag_(nullptr), bagwidth_(0), bagheight_(0)
{
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

	element_=SharedPtr<UIElement>(new UIElement(context_));
	element_->SetSize(IntVector2(320, 480));

	auto slotelement=element_->CreateChild<UIElement>("SlotElement");
	slotelement->SetSize(IntVector2(256, 256));
	slotelement->SetPosition(IntVector2(32,0));

	// Build slots
	BuildSlot(0,0,2,3,"Blade");
	BuildSlot(96,0,2,2,"Turret");
	BuildSlot(192,0,2,3,"Blade;Shield");
	BuildSlot(0,128,2,2,"DriveSystem");
	BuildSlot(96,96,2,3,"Shell");
	BuildSlot(192,128,2,2,"GeneratorSystem");
	for(unsigned int c=0; c<8; ++c)
	{
		BuildSlot(c*32,226,1,1,"Processor");
	}

	BuildBag(10, 6);
	element_->SetPosition(IntVector2(graphics->GetWidth()/2, graphics->GetHeight()/4));
	element_->SetVisible(false);
	ui->GetRoot()->GetChild("HUDLayer", true)->AddChild(element_);
}

void InventoryScreen::BuildBag(unsigned int width, unsigned int height)
{
	auto cache=GetSubsystem<ResourceCache>();

	if(element_)
	{
		bag_=element_->CreateChild<UIElement>("Bag");
		bag_->SetSize(width*32, height*32);
		bagwidth_=width;
		bagheight_=height;
		bag_->SetPosition(IntVector2(0,288));

		for(unsigned int y=0; y<height; ++y)
		{
			for(unsigned int x=0; x<width; ++x)
			{
				auto slot=bag_->CreateChild<BorderImage>("Background");
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
				bagslotsblocked_.push_back(false);

			}
		}
	}

}

void InventoryScreen::BuildSlot(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const String &allowedtypes)
{
	if(!element_) return;

	auto pd=GetSubsystem<PlayerData>();
	auto cache=GetSubsystem<ResourceCache>();

	auto &eqs=pd->GetEquipmentSet();
	auto slotelement=element_->GetChild("SlotElement", true);
	if(!slotelement) return;

	auto sl=slotelement->CreateChild<BorderImage>("Background");
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

	unsigned int es=eqs.CreateSlot(allowedtypes);
	//equipmentsetslots_.push_back(es);
	sl->SetVar(StringHash("slot"), es);
	slots_.push_back(sl);

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

bool InventoryScreen::CanEquipItemInSlot(unsigned int slot, EquipmentItemDef &def)
{
	auto pd=GetSubsystem<PlayerData>();
	auto &eqp=pd->GetEquipmentSet();

	if(slot>=slots_.size()) return false;
	return eqp.CanAddItemToSlot(slot, def);
}

IntVector2 InventoryScreen::GetInventorySizeByType(EquipmentSlots slot)
{
	switch(slot)
	{
		case EqBlade: return IntVector2(2,3); break;
		case EqTurret: return IntVector2(2,2); break;
		case EqShell: return IntVector2(2,3); break;
		case EqProcessor: return IntVector2(1,1); break;
		case EqDriveSystem: return IntVector2(2,2); break;
		case EqGeneratorSystem: return IntVector2(2,2); break;
		case EqShield: return IntVector2(2,3); break;
		default: return IntVector2(0,0); break;
	};
}

UIElement *InventoryScreen::GetHoveredSlot(const IntVector2 &mousepos, unsigned int &slot)
{
	// Get which equipment slot is being hovered over, return nullptr if none
	if(!element_) return nullptr;
	auto ui=GetSubsystem<UI>();

	for(unsigned int c=0; c<slots_.size(); ++c)
	{
		auto slt = slots_[c];
		if(slt)
		{
			IntRect rect=slt->GetCombinedScreenRect();
			if(rect.IsInside(mousepos)==INSIDE)
			{
				slot=c;
				return slt;
			}
		}
	}
	return nullptr;
}

// Get which bag slot the mouse is over
bool InventoryScreen::GetHoveredBagSlot(const IntVector2 &mousepos, IntVector2 &pos)
{
	if(!bag_) return false;

	auto bagrect=bag_->GetCombinedScreenRect();
	if(bagrect.IsInside(mousepos)==INSIDE)
	{
		IntVector2 relpos=mousepos-bag_->GetPosition();
		unsigned int sx=relpos.x_/32;
		unsigned int sy=relpos.y_/32;
		unsigned int index=sy*bagwidth_+sx;
		if(index>=bagslots_.size()) return false;
		pos.x_=sx;
		pos.y_=sy;
		return true;
	}
	return false;
}

bool InventoryScreen::FindBagPosition(IntVector2 &slotpos, const IntVector2 &size)
{
	for(unsigned int x=0; x<bagwidth_ - size.x_; ++x)
	{
		for(unsigned int y=0; y<bagheight_ - size.y_; ++y)
		{
			bool blocked=false;
			for(unsigned int k=0; k<size.x_; ++k)
			{
				for(unsigned int l=0; l<size.y_; ++l)
				{
					unsigned int cx=x+k;
					unsigned int cy=y+l;
					unsigned int index=cy*bagwidth_+cx;
					if(index>=bagslotsblocked_.size() || bagslotsblocked_[index]==true) blocked=true;
				}
			}
			if(!blocked)
			{
				slotpos.x_=x;
				slotpos.y_=y;
				return true;
			}
		}
	}
	return false;
}

// Check to see if we can place an item of given size at slot.

bool InventoryScreen::CanPlaceItemInSlot(const IntVector2 &pos, const IntVector2 &size, bool docolor)
{
	if(pos.x_<0 || pos.y_<0 || pos.x_>=bagwidth_-size.x_ || pos.y_>=bagheight_-size.y_) return false;
	bool blocked=false;
	for(unsigned int x=0; x<size.x_; ++x)
	{
		for(unsigned int y=0; y<size.y_; ++y)
		{
			unsigned int cx=pos.x_+x;
			unsigned int cy=pos.y_+y;
			unsigned int index=cy*bagwidth_+cx;
			if(index>=bagslotsblocked_.size() || bagslotsblocked_[index]==true) blocked=true;
		}
	}

	if(docolor)
	{
		for(unsigned int x=0; x<bagwidth_; ++x)
		{
			for(unsigned int y=0; y<bagheight_; ++y)
			{
				unsigned int index=y*bagwidth_+x;
				auto slot=bagslots_[index];
				if(x<pos.x_ || x>=pos.x_+size.x_ || y<pos.y_ || y>=pos.y_+size.y_)
				{
					slot->GetChild("Background",true)->SetColor(Color(0.5,0.5,0.5));
				}
				else
				{
					if(blocked)
					{
						slot->GetChild("Background",true)->SetColor(Color(1,0.5,0.5));
					}
					else
					{
						slot->GetChild("Background",true)->SetColor(Color(0.5,1,0.5));
					}
				}
			}
		}
	}
	return blocked;
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

GeneralItem *InventoryScreen::GetHoveredBagItem(const IntVector2 &mousepos, IntVector2 &slot)
{
	IntVector2 hoveredslot;
	if(!GetHoveredBagSlot(mousepos,hoveredslot)) return nullptr;

	for(unsigned int i=0; i<itemsinbag_.size(); ++i)
	{
		auto item = itemsinbag_[i];
		if(item && !item.Expired())
		{

		}
	}

	return nullptr;
}
