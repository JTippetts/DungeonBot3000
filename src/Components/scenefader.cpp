#include "scenefader.h"

#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>

void SceneFader::RegisterObject(Context *context)
{
	context->RegisterFactory<SceneFader>();
}

SceneFader::SceneFader(Context *context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void SceneFader::SetFadeDuration(float dur)
{
	duration_=dur;
}

void SceneFader::SetFadeState(States state)
{
	state_=state;
	counter_=0.0f;
}

void SceneFader::Update(float dt)
{
	if(state_==FadedIn || state_==FadedOut) return;

	if(state_==FadingIn)
	{
		if(!quad_) Setup();
		counter_+=dt;
		if(counter_>=duration_)
		{
			// Fully faded in, send event and exit
			VariantMap vm;
			SendEvent(StringHash("FadedIn"), vm);
			SetFadeState(FadedIn);
			quad_->Remove();
			quad_.Reset();
			return;
		}
		float opacity=1.0f-counter_/duration_;
		quad_->SetOpacity(opacity);
	}
	else if(state_==FadingOut)
	{
		if(!quad_) Setup();
		counter_+=dt;
		if(counter_>=duration_)
		{
			// Fully faded out
			VariantMap vm;
			SendEvent(StringHash("FadedOut"),vm);
			SetFadeState(FadedOut);
			quad_->Remove();
			quad_.Reset();
			return;
		}
		float opacity=counter_/duration_;
		quad_->SetOpacity(opacity);
		Log::Write(LOG_INFO, String("Opacity: ") + String(opacity));
	}
}

void SceneFader::Setup()
{
	auto cache=GetSubsystem<ResourceCache>();
	auto ui=GetSubsystem<UI>();
	auto graphics=GetSubsystem<Graphics>();

	quad_=new BorderImage(context_);
	quad_->SetTexture(cache->GetResource<Texture2D>("Textures/UI.png"));
	quad_->SetImageRect(IntRect(87,92,88,93));
	quad_->SetColor(Color(0,0,0));
	quad_->SetOpacity(0);
	quad_->SetEnabled(true);
	quad_->SetVisible(true);

	quad_->SetSize(graphics->GetSize());


	ui->GetRoot()->AddChild(quad_);

}
