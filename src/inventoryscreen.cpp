#include "inventoryscreen.h"
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Core/StringUtils.h>

#include "playerdata.h"
#include "playerinventory.h"

InventoryScreen::InventoryScreen(Context *context) : Object(context), bagelement_(nullptr), equipelement_(nullptr), bagwidth_(0), bagheight_(0)
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
	if(pi->GetBag().IsDirty())
	{
		UpdateBagImages();
	}
	if(pi->GetEquipment().IsDirty())
	{
		UpdateEquipmentImages();
	}
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
