#include "dropitem.h"

void DropItemContainer::RegisterObject(Context *context)
{
	context->RegisterFactory<DropItemContainer>();
}

DropItemContainer::DropItemContainer(Context *context) : LogicComponent(context)
{
}

void DropItemContainer::SetItem(const EquipmentItemDef &item)
{
	item_=item;
}

const EquipmentItemDef &DropItemContainer::GetItem() const
{
	return item_;
}
