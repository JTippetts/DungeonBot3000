#pragma once

// Master list of item mods

#include <vector>
#include <unordered_map>
#include <Urho3D/Urho3D.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>
#include "stats.h"

using namespace Urho3D;

enum ItemModDesignation
{
	IMImplicit,
	IMLocal,
	IMGlobal
};

struct ItemModEntry
{
	String desc_;
	StatSet statset_;
	ItemModDesignation desig_;

	ItemModEntry() : desig_(IMGlobal)
	{
	}

	ItemModEntry(String d, StatSet &ss, ItemModDesignation des) : desc_(d), statset_(ss), desig_(des)
	{
	}
};

using ItemModMap=std::unordered_map<std::string, ItemModEntry>;

class ItemModTable
{
	public:
	ItemModTable();

	void LoadJSON(const JSONValue &json);

	ItemModEntry *GetMod(const String &name);
	ItemModEntry *GetMod(const std::string &name);
	protected:
	ItemModMap mods_;
};

struct ItemModTierSet
{
	int minlevel_;
	std::vector<String> modlist_;
};

using ItemModTierSetTable=std::vector<ItemModTierSet>;


// ItemModTierTable
// Implement a weighted table to choose an item mod from an array of same-type mods.
// Mods are organized into tier lists, and weighted according to some exponent so that higher tiers
// are more rare.
// Data for this class is found in the entries within Tables/Items/itemmodtiers.json

class ItemModTierTable
{
	public:
	ItemModTierTable();

	void LoadJSON(const JSONValue &json);

	String Choose(int level);

	protected:
	ItemModTierSetTable table_;
	double weighting_;
};

using ItemModTierTableMap=std::unordered_map<std::string, ItemModTierTable>;

// Implement a class to hold all sets of item mod tiers

class ItemModTiers
{
	public:
	ItemModTiers();

	void LoadJSON(const JSONValue &json);

	String Choose(const String &which, int level);
	String Choose(const std::string &which, int level);

	protected:
	ItemModTierTableMap map_;
};

//Class to hold an entire set of base classes for an equipment slot category (blades, turrets, etc...)

struct ItemClassEntry
{
	String name_;
	int minlevel_;
	double weight_;
	std::vector<String> fixed_;
	std::vector<String> random_;
};

class ItemClass
{
	public:
	ItemClass(){}

	void LoadJSON(const JSONValue &json);
	ItemClassEntry *Choose(int level);

	protected:
	std::vector<ItemClassEntry> entries_;
};
