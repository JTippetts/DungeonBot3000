#include "equipmentset.h"

#include "playerdata.h"

EquipmentEntry::EquipmentEntry() : used_(false), active_(true)
{
}

EquipmentEntry::EquipmentEntry(const String &allowables) : EquipmentEntry()
{
/*
enum EquipmentSlots
{
	EqBlade,
	EqTurret,
	EqShell,
	EqProcessor,
	EqDriveSystem,
	EqGeneratorSystem,
	EqShield,
	EqNumEquipmentSlots
};
*/
	auto all = allowables.Split(';');
	for(unsigned int c=0; c<all.Size(); ++c)
	{
		String &s = all[c];
		if(s=="Blade") allowabletypes_.push_back(EqBlade);
		if(s=="Turret") allowabletypes_.push_back(EqTurret);
		if(s=="Shell") allowabletypes_.push_back(EqShell);
		if(s=="Processor") allowabletypes_.push_back(EqProcessor);
		if(s=="DriveSystem") allowabletypes_.push_back(EqDriveSystem);
		if(s=="GeneratorSystem") allowabletypes_.push_back(EqGeneratorSystem);
		if(s=="Shield") allowabletypes_.push_back(EqShield);
	}
}

bool EquipmentEntry::Allowed(EquipmentSlots slot)
{
	for(auto i : allowabletypes_)
	{
		if(slot==i) return true;
	}
	return false;
}

EquipmentSet::EquipmentSet(Context *context) : Object(context)
{
}

unsigned int EquipmentSet::CreateSlot(const String &allowables)
{
	slots_.push_back(EquipmentEntry(allowables));
	localstats_.push_back(StatSet());

	return slots_.size()-1;
}

bool EquipmentSet::CanAddItemToSlot(unsigned int slot, const EquipmentItemDef &def)
{
	if(slot>=slots_.size()) return false;

	auto &s=slots_[slot];
	if (s.Allowed(def.slot_)) return true;

	return false;
}

bool EquipmentSet::AddItemToSlot(EquipmentItemDef &existing, unsigned int slot, const EquipmentItemDef &def)
{
	// This method returns true if there was already an item in the slot, and fills in existing with the item there
	if(!CanAddItemToSlot(slot, def)) return false;

	auto pd=GetSubsystem<PlayerData>();
	auto &itemmodtable = pd->GetItemModTable();

	auto &s=slots_[slot];
	bool hasexisting=false;
	if(s.used_)
	{
		hasexisting=true;
		existing=s.def_;
	}
	s.def_=def;
	s.used_=true;

	localstats_[slot].Clear();
	for(auto m : def.itemmods_)
	{
		auto mod = itemmodtable.GetMod(m);
		if(mod)
		{
			if(mod->desig_ == IMImplicit || mod->desig_ == IMLocal)
			{
				// Add a local mod
				localstats_[slot].Merge(mod->statset_);
			}
		}
	}
	RebuildGlobalStatSet();
	return hasexisting;
}

bool EquipmentSet::RemoveItemFromSlot(EquipmentItemDef &existing, unsigned int slot)
{
	if(slot>=slots_.size())
	{
		return false;
	}

	auto &s=slots_[slot];
	bool hasexisting=false;
	if(s.used_)
	{
		hasexisting=true;
		existing=s.def_;
	}
	localstats_[slot].Clear();
	s.used_=false;
	RebuildGlobalStatSet();
	return hasexisting;
}

bool EquipmentSet::GetItemAtSlot(EquipmentItemDef &existing, unsigned int slot)
{
	if(slot>=slots_.size())
	{
		return false;
	}

	auto &s=slots_[slot];
	bool hasexisting=false;
	if(s.used_)
	{
		hasexisting=true;
		existing=s.def_;
	}
	return hasexisting;
}

StatSet *EquipmentSet::GetGlobalStats()
{
	return &globalstats_;
}

StatSet *EquipmentSet::GetImplicitStats(unsigned int slot)
{
	if(slot>=localstats_.size())
	{
		return nullptr;
	}
	return &localstats_[slot];
}

unsigned int EquipmentSet::GetImplicitStats(StatSetCollection &ssc, EquipmentSlots eqtype)
{
	unsigned int count=0;
	for(unsigned int c=0; c<slots_.size(); ++c)
	{
		auto &s=slots_[c];
		if(s.used_ && s.active_ && s.Allowed(eqtype))
		{
			ssc.push_back(&localstats_[c]);
			++count;
		}
	}
	return count;
}

unsigned int EquipmentSet::GetNumberOfSlots()
{
	return slots_.size();
}

void EquipmentSet::RebuildGlobalStatSet()
{
	auto pd=GetSubsystem<PlayerData>();
	auto &itemmodtable = pd->GetItemModTable();

	globalstats_.Clear();
	for(unsigned int c=0; c<slots_.size(); ++c)
	{
		auto &s = slots_[c];
		if(s.used_ && s.active_)
		{
			for(auto m : s.def_.itemmods_)
			{
				auto mod = itemmodtable.GetMod(m);
				if(mod)
				{
					if(mod->desig_ == IMGlobal)
					{
						// Add a local mod
						globalstats_.Merge(mod->statset_);
					}
				}
			}
		}
	}
}

void EquipmentSet::SetSlotActive(unsigned int slot, bool a)
{
	if(slot>=slots_.size()) return;

	auto &s=slots_[slot];
	s.active_=a;
	RebuildGlobalStatSet();
}
