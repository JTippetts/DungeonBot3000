#include "scenefader.h"

#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>

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
	if(!quad_) return;
	if(state_==FadingOut)
	{
		// Set opacity to 0 and enable
		quad_->SetEnabled(true);
		quad_->SetVisible(true);
		quad_->SetOpacity(0);
	}
	else if(state_==FadingIn)
	{
		quad_->SetEnabled(true);
		quad_->SetVisible(true);
		quad_->SetOpacity(1.0f);
	}
	else
	{
		quad_->SetEnabled(false);
		quad_->SetVisible(false);
	}
}

void SceneFader::Update(float dt)
{
	if(state_==FadedIn || state_==FadedOut) return;
	if(!quad_) return;

	if(state_==FadingIn)
	{
		counter_+=dt;
		if(counter_>=duration_)
		{
			// Fully faded in, send event and exit
			VariantMap vm;
			SendEvent(StringHash("FadedIn"), vm);
			SetFadeState(FadedIn);
			return;
		}
		float opacity=1.0f-counter_/duration_;
		quad_->SetOpacity(opacity);
	}
	else if(state_==FadingOut)
	{
		counter_+=dt;
		if(counter_>=duration_)
		{
			// Fully faded out
			VariantMap vm;
			SendEvent(StringHash("FadedOut"),vm);
		}
	}
}

void SceneFader::Start()
{
	auto cache=GetSubsystem<ResourceCache>();
	auto ui=GetSubsystem<UI>();
	auto graphics=GetSubsystem<Graphics>();

	quad_=new BorderImage(context_);
	quad_->SetTexture(cache->GetResource<Texture2D>("Textures/UI.png"));
	quad_->SetImageRect(IntRect(87,92,88,93));
	quad_->SetColor(Color(0,0,0));
	quad_->SetOpacity(0);
	quad_->SetEnabled(false);
	quad_->SetVisible(false);

	quad_->SetSize(graphics->GetSize());


	ui->GetRoot()->AddChild(quad_);

}
