#include "dropitem.h"

void DropItemContainer::RegisterObject(Context *context)
{
	context->RegisterFactory<DropItemContainer>();
}

DropItemContainer::DropItemContainer(Context *context) : LogicComponent(context)
{
}

void DropItemContainer::SetItem(GeneralItem *item)
{
	item_=item;
}

GeneralItem *DropItemContainer::GetItem()
{
	if(!item_.Expired()) return item_;
	return nullptr;
}
