#include "playerinventory.h"
#include <Urho3D/IO/Log.h>

InventoryBag::InventoryBag() : dirty_(false)
{
}

void InventoryBag::Setup(unsigned int width, unsigned int height)
{
	blocked_.resize(width*height);
	for(unsigned int c=0; c<blocked_.size(); ++c) blocked_[c]=false;
	bagwidth_=width;
	bagheight_=height;
}

bool InventoryBag::FindAvailableLocation(IntVector2 &location, const IntVector2 &size)
{
	if(location.x_<0 || location.y_<0 || location.x_>=bagwidth_-size.x_ || location.y_>=bagheight_-size.y_) return false;

	for(unsigned int x=0; x<=bagwidth_ - size.x_; ++x)
	{
		for(unsigned int y=0; y<=bagheight_ - size.y_; ++y)
		{
			bool blocked=false;
			for(unsigned int k=0; k<size.x_; ++k)
			{
				for(unsigned int l=0; l<size.y_; ++l)
				{
					unsigned int cx=x+k;
					unsigned int cy=y+l;
					unsigned int index=cy*bagwidth_+cx;
					if(index>=blocked_.size() || blocked_[index]==true) blocked=true;
				}
			}
			if(!blocked)
			{
				location.x_=x;
				location.y_=y;
				return true;
			}
		}
	}
	return false;
}

bool InventoryBag::FindAvailableLocation(IntVector2 &location, GeneralItem *item)
{
	if(!item) return false;
	IntVector2 size=item->invsize_;
	return FindAvailableLocation(location, size);
}

bool InventoryBag::CanPlaceAtLocation(const IntVector2 &location, GeneralItem *item, bool checkblock)
{
	if(!item) return false;
	IntVector2 size=item->invsize_;
	if(location.x_<0 || location.y_<0 || location.x_>=bagwidth_-size.x_ || location.y_ >=bagheight_-size.y_) return false;

	if(checkblock)
	{
		bool blocked=false;
		for(unsigned int x=location.x_; x<location.x_+size.x_; ++x)
		{
			for(unsigned int y=location.y_; y<location.y_+size.y_; ++y)
			{
				unsigned int index=y*bagwidth_+x;
				if(index>=blocked_.size()) return false;
				if(blocked_[index]) blocked=true;
			}
		}
		return blocked;
	}
	else return true;
}

void InventoryBag::PlaceAtLocation(const IntVector2 &location, GeneralItem *item)
{
	if(!item) return;
	items_.push_back(WeakPtr<GeneralItem>(item));
	IntVector2 size=item->invsize_;
	item->invlocation_ = location;

	for(unsigned int x=location.x_; x<location.x_+size.x_; ++x)
	{
		for(unsigned int y=location.y_; y<location.y_+size.y_; ++y)
		{
			unsigned int index=y*bagwidth_+x;
			if(index<blocked_.size()) blocked_[index]=true;
		}
	}
	dirty_=true;
}

GeneralItem *InventoryBag::FindItemInSlot(const IntVector2 &location)
{
	for(unsigned int c=0; c<items_.size(); ++c)
	{
		auto item=items_[c];
		IntVector2 &invsize=item->invsize_;
		IntVector2 &invloc=item->invlocation_;
		if(location.x_>=invloc.x_ && location.x_<invloc.x_+invsize.x_ && location.y_>=invloc.y_ && location.y_<invloc.y_ + invsize.y_) return item;
	}
	return nullptr;
}

void InventoryBag::RemoveItem(GeneralItem *item)
{
	if(!item) return;
	for(unsigned int i=0; i<items_.size(); ++i)
	{
		if(items_[i]==item)
		{
			IntVector2 &invsize=item->invsize_;
			IntVector2 &invloc=item->invlocation_;
			for(unsigned int x=0; x<invsize.x_; ++x)
			{
				for(unsigned int y=0; y<invsize.y_; ++y)
				{
					unsigned int cx=x+invloc.x_;
					unsigned int cy=y+invloc.y_;
					unsigned int index=cy*bagwidth_+cx;
					if(index<blocked_.size())
					{
						blocked_[index]=false;
					}
				}
			}
			items_[i]=items_[items_.size()-1];
			items_.pop_back();
			dirty_=true;
			return;
			//items_.erase(i);
		}
	}
	dirty_=true;
}

// Count the number of items covered by the given location+size
unsigned int InventoryBag::CountCoveredItems(const IntVector2 &location, const IntVector2 &size)
{
	std::vector<GeneralItem *> items;

	for(unsigned int x=location.x_; x<location.x_ + size.x_; ++x)
	{
		for(unsigned int y=location.y_; y<location.y_ + size.y_; ++y)
		{
			auto i=FindItemInSlot(IntVector2(x,y));
			if(i)
			{
				bool found=false;
				for(auto item : items)
				{
					if(i==item) found=true;
				}
				if(!found) items.push_back(i);
			}
		}
	}

	return items.size();
}

GeneralItem * InventoryBag::GetCoveredItem(const IntVector2 &location, const IntVector2 &size)
{
	for(unsigned int x=location.x_; x<location.x_ + size.x_; ++x)
	{
		for(unsigned int y=location.y_; y<location.y_ + size.y_; ++y)
		{
			auto i=FindItemInSlot(IntVector2(x,y));
			if(i)
			{
				return i;
			}
		}
	}
	return nullptr;
}

bool InventoryBag::IsDirty()
{
	bool dirty=dirty_;
	dirty_=false;
	return dirty;
}

PlayerInventory::PlayerInventory(Context *context) : Object(context), equipment_(context)
{
}

bool PlayerInventory::CanEquipItemInSlot(unsigned int slot, GeneralItem *item, bool checkused)
{
	return equipment_.CanAddItemToSlot(slot, item, checkused);
}

bool PlayerInventory::CanEquipItemInSlot(StringHash slot, GeneralItem *item, bool checkused)
{
	auto i = equipmentslots_.Find(slot);
	if(i!=equipmentslots_.End())
	{
		return CanEquipItemInSlot((*i).second_, item, checkused);
	}
	return false;
}

bool PlayerInventory::CanPlaceItemInBag(IntVector2 &pos, const IntVector2 &size)
{
	bool found=bag_.FindAvailableLocation(pos, size);
	return found;
}

bool PlayerInventory::CanPlaceItemInBag(GeneralItem *item)
{
	if(!item) return false;
	IntVector2 &size=item->invsize_;
	IntVector2 location;
	bool found=bag_.FindAvailableLocation(location, size);
	return found;
}

bool PlayerInventory::AddItem(GeneralItem *item, bool equipifable)
{
	if(!item) return false;
	if(equipifable)
	{
		if(item->type_==GITEquipment)
		{
			// Find an empty slot that matches slot type
			unsigned int numslots=equipment_.GetNumberOfSlots();
			for(unsigned int c=0; c<numslots; ++c)
			{
				if(equipment_.CanAddItemToSlot(c, item, true))
				{
					if(equipment_.AddItemToSlot(c, item))
					{
						Log::Write(LOG_INFO, "Item equipped automaticall");
						return true;
					}
				}
			}
		}
	}

	// No equip, add to bag
	IntVector2 loc;
	bool found=bag_.FindAvailableLocation(loc, item);
	if(!found)
	{
		return false;
	}
	bag_.PlaceAtLocation(loc, item);
	return true;
}

bool PlayerInventory::GetSlot(StringHash name, unsigned int &slot)
{
	auto i=equipmentslots_.Find(name);
	if(i!=equipmentslots_.End())
	{
		slot = (*i).second_;
		return true;
	}
	return false;
}

void PlayerInventory::AddEquipmentSlot(StringHash name, const String &allowed)
{
	equipmentslots_[name]=equipment_.CreateSlot(allowed);
}

InventoryBag &PlayerInventory::GetBag()
{
	return bag_;
}

EquipmentSet &PlayerInventory::GetEquipment()
{
	return equipment_;
}
