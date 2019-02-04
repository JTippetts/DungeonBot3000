#include "itemnametag.h"
#include "thirdpersoncamera.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/UI.h>

void ItemNameTag::RegisterObject(Context *context)
{
	context->RegisterFactory<ItemNameTag>("UI");
}

ItemNameTag::ItemNameTag(Context *context) : LogicComponent(context), element_(nullptr)
{
}

IntVector2 ItemNameTag::GetObjectScreenLocation()
{
	auto cam=node_->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
	auto graphics=GetSubsystem<Graphics>();
	IntVector2 screenpos;
	if(cam)
	{
		Vector3 headpos=node_->GetWorldPosition();
		Vector2 pos=cam->WorldToScreen(headpos);
		float sx = pos.x_ * (float)graphics->GetWidth();
		float sy = pos.y_ * (float)graphics->GetHeight();
		screenpos=IntVector2((int)sx, (int)sy);
	}
	return screenpos;
}

IntVector2 ItemNameTag::GetScreenLocation()
{
	return screenlocation_;
}

IntVector2 ItemNameTag::GetScreenSize()
{
	if(element_ )
	{
		return element_->GetSize();
	}
	return IntVector2(0,0);
}

void ItemNameTag::SetScreenLocation(const IntVector2 &loc)
{
	IntVector2 size=GetScreenSize();
	screenlocation_=loc - size/2;
	if(element_) element_->SetPosition(screenlocation_);
}

void ItemNameTag::SetItemName(const String &name)
{
	if(!element_) return;
	auto txt = element_->GetChildDynamicCast<Text>("NameTag", true);
	if(txt)
	{
		txt->SetText(name);
		Log::Write(LOG_INFO, String("Name set to ") + name);
	}
	else
	{
		Log::Write(LOG_INFO, "Could not set item name tag.");
	}
}

void ItemNameTag::SetItemColor(const Color &c)
{
	if(!element_) return;
	auto txt = element_->GetChildDynamicCast<Text>("NameTag", true);
	if(txt)
	{
		txt->SetColor(c);
		Log::Write(LOG_INFO, "Color set");
	}
}

void ItemNameTag::Start()
{
	IntVector2 screenpos=GetObjectScreenLocation();

	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();

	element_ = ui->LoadLayout(cache->GetResource<XMLFile>("UI/ItemNameTag.xml"));
	ui->GetRoot()->AddChild(element_);
	//element_->SetVisible(false);
	SetScreenLocation(screenpos);
	element_->SetVar(StringHash("ItemNameTag"), Variant(this));
}

void ItemNameTag::Update(float dt)
{
	SetScreenLocation(GetObjectScreenLocation());
}

void ItemNameTag::Stop()
{
	if(element_) element_->Remove();
}
