#include "scenefade.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Graphics.h>

SceneFade::SceneFade(Context *context) : Object(context), duration_(0.0f), counter_(0.0f)
{
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(SceneFade, HandleUpdate));
}

bool SceneFade::IsFading()
{
	return (counter_<duration_);
}

void SceneFade::FadeIn(float duration)
{
	if(IsFading()) return;

	duration_=duration;
	counter_=0.0f;
	fadein_=true;

	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();

	element_=ui->GetRoot()->CreateChild<BorderImage>();
	element_->SetTexture(cache->GetResource<Texture2D>("Textures/UI.png"));
	element_->SetImageRect(IntRect(86,90,87,91));
	element_->SetColor(Color(0,0,0));
	element_->SetOpacity(1.0);
}

void SceneFade::FadeOut(float duration)
{
	if(IsFading()) return;

	duration_=duration;
	counter_=0.0f;
	fadein_=true;

	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();
	auto graphics=GetSubsystem<Graphics>();

	element_=ui->GetRoot()->CreateChild<BorderImage>();
	element_->SetTexture(cache->GetResource<Texture2D>("Textures/UI.png"));
	element_->SetImageRect(IntRect(86,90,87,91));
	element_->SetColor(Color(0,0,0));
	element_->SetOpacity(1.0);
}

void SceneFade::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
	if(!IsFading()) return;

	static StringHash TimeStep("TimeStep");
	float dt=eventData[TimeStep].GetFloat();

	float opacity;
	counter_ += dt;
	if(counter_>=duration_)
	{
		element_->Remove();
		element_.Reset();
	}
}
