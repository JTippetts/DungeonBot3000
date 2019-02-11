#include "enemynameplate.h"

#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>

#include "vitals.h"

void EnemyNameplate::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyNameplate>();

	URHO3D_ACCESSOR_ATTRIBUTE("Name", GetName, SetName, String, "Enemy Name", AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Name Color", GetNameColor, SetNameColor, Color, Color(1,1,1,1), AM_DEFAULT);
}

EnemyNameplate::EnemyNameplate(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void EnemyNameplate::SetName(const String &name)
{
	name_=name;
	if(element_)
	{
		auto nm=element_->GetChildDynamicCast<Text>("Name", true);
		if(nm) nm->SetText(name_);
	}
}

const String &EnemyNameplate::GetName() const
{
	return name_;
}

void EnemyNameplate::SetNameColor(const Color &color)
{
	color_=color;
	if(element_)
	{
		auto nm=element_->GetChildDynamicCast<Text>("Name", true);
		if(nm) nm->SetColor(color_);
	}
}

const Color &EnemyNameplate::GetNameColor() const
{
	return color_;
}

void EnemyNameplate::SetHealthbar(float ratio)
{
	if(element_)
	{
		auto bar=element_->GetChildDynamicCast<BorderImage>("LifeBar", true);
		if(bar)
		{
			bar->SetWidth(ratio * 256.0f);
		}
	}
}

void EnemyNameplate::Start()
{
	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();

	element_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/EnemyNameplate.xml"));
	ui->GetRoot()->AddChild(element_);
	element_->SetVisible(false);
	element_->SetPosition(IntVector2(graphics->GetWidth()/2 - element_->GetWidth()/2, 0));
}

void EnemyNameplate::Update(float dt)
{
	auto vitals = node_->GetDerivedComponent<BaseVitals>();
	if(vitals)
	{
		SetHealthbar(vitals->GetCurrentLife() / vitals->GetMaximumLife());
	}
}

void EnemyNameplate::DelayedStart()
{
	SubscribeToEvent(node_, StringHash("Hover"), URHO3D_HANDLER(EnemyNameplate, HandleHover));
	SubscribeToEvent(node_, StringHash("UnHover"), URHO3D_HANDLER(EnemyNameplate, HandleUnHover));
}

void EnemyNameplate::SetVisible(bool vis)
{
	if(element_) element_->SetVisible(vis);
}

void EnemyNameplate::HandleHover(StringHash eventType, VariantMap &eventData)
{
	SetVisible(true);
}

void EnemyNameplate::HandleUnHover(StringHash eventType, VariantMap &eventData)
{
	SetVisible(false);
}

void EnemyNameplate::Stop()
{
	if(element_)
	{
		element_->Remove();
		element_.Reset();
	}
}
