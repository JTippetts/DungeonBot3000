#include "enemylifebar.h"

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/BorderImage.h>

#include "thirdpersoncamera.h"
#include "vitals.h"


void EnemyLifeBar::RegisterObject(Context *context)
{
	context->RegisterFactory<EnemyLifeBar>("UI");
	URHO3D_ACCESSOR_ATTRIBUTE("Offset", GetOffset, SetOffset, double, 1.0, AM_DEFAULT);
}

EnemyLifeBar::EnemyLifeBar(Context *context) : LogicComponent(context), offset_(1.0)
{
}

void EnemyLifeBar::SetOffset(double f)
{
	offset_=f;
}

const double EnemyLifeBar::GetOffset() const
{
	return offset_;
}

void EnemyLifeBar::DelayedStart()
{
	auto ui=GetSubsystem<UI>();
	auto cache=GetSubsystem<ResourceCache>();

	element_ = ui->LoadLayout(cache->GetResource<XMLFile>("UI/enemybar.xml"));
	ui->GetRoot()->AddChild(element_);
	element_->SetVisible(false);
}

void EnemyLifeBar::Update(float dt)
{
	if(!element_) return;

	auto cam=node_->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
	auto graphics=GetSubsystem<Graphics>();
	IntVector2 screenpos;
	if(cam)
	{
		Vector3 headpos=node_->GetWorldPosition() + Vector3(0, offset_, 0);
		Vector2 pos=cam->WorldToScreen(headpos);
		float sx = pos.x_ * (float)graphics->GetWidth();
		float sy = pos.y_ * (float)graphics->GetHeight();
		screenpos=IntVector2((int)sx, (int)sy);
	}

	element_->SetPosition(screenpos + IntVector2(-32,-2));

	// Update bar
	auto vtls = node_->GetComponent<EnemyVitals>();
	if(vtls)
	{
		double curlife=vtls->GetCurrentLife();
		double maxlife=vtls->GetMaximumLife();

		if(curlife<maxlife)
		{
			if(screenpos.x_ > -element_->GetWidth()/2 && screenpos.x_ < graphics->GetWidth()+element_->GetWidth()/2 &&
				screenpos.y_ > -element_->GetHeight()/2 && screenpos.y_ < graphics->GetHeight()+element_->GetHeight()/2)
			{
				element_->SetVisible(true);

				double ratio=curlife/maxlife;
				int width=(int)(ratio * (float)element_->GetWidth());
				auto bar=element_->GetChild("Bar", true);
				if(bar)
				{
					bar->SetWidth(width);
				}
			}
			else
			{
				element_->SetVisible(false);
			}
		}
		else
		{
			element_->SetVisible(false);
		}
	}

	// Testing
	//element_->SetVisible(true);
}
