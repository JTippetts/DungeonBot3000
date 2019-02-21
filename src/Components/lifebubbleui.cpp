#include "lifebubbleui.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/RenderSurface.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Input/Input.h>

#include "vitals.h"
#include "../playerdata.h"
#include "../UIMesh.h"

using namespace Urho3D;

float rollf(float,float);
int roll(int,int);

void LifeBubbleUI::RegisterObject(Context *context)
{
	context->RegisterFactory<LifeBubbleUI>("UI");
}

LifeBubbleUI::LifeBubbleUI(Context *context) : LogicComponent(context)
{
}

void LifeBubbleUI::DelayedStart()
{
	// Set up the RTT scene
	auto cache=GetSubsystem<ResourceCache>();

	auto ui=GetSubsystem<UI>();
	auto graphics=GetSubsystem<Graphics>();
	healthmat_=cache->GetResource<Material>("Materials/health.xml");
	element_ = ui->LoadLayout(cache->GetResource<XMLFile>("UI/healthbubble.xml"));


	auto bubble = dynamic_cast<UIMesh *>(element_->GetChild("Bubble", true));
	if(bubble)
	{
		Log::Write(LOG_INFO, "Setting bubble values");
		bubble->SetSize(127,127);
		bubble->SetPosition(64,64);
	}
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(element_);
	element_->SetPosition(IntVector2(-38, graphics->GetHeight()-256+38));

	auto pd=GetSubsystem<PlayerData>();
	energyelement_=new Text(context_);
	energyelement_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"));
	energyelement_->SetFontSize(24);
	energyelement_->SetText(String("Energy: ") + String((int)pd->GetEnergy()));
	energyelement_->SetColor(Color(0.25,1.0,0.25));
	energyelement_->SetPosition(IntVector2(0, graphics->GetHeight()-256+38-energyelement_->GetHeight()));
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(energyelement_);

	help_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/Help.xml"));
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(help_);
	help_->SetPosition(IntVector2(graphics->GetWidth()-help_->GetWidth(), graphics->GetHeight()-help_->GetHeight()));

	levelelement_=new Text(context_);
	levelelement_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"));
	levelelement_->SetFontSize(24);
	levelelement_->SetText(String("Dungeon Level ") + String((int)pd->GetDungeonLevel()));
	levelelement_->SetColor(Color(0.25,1.0,0.75));
	levelelement_->SetPosition(IntVector2(0, 0));
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(levelelement_);

	gamblebutton_=ui->LoadLayout(cache->GetResource<XMLFile>("UI/GambleButton.xml"));
	ui->GetRoot()->GetChild("HUDLayer",true)->AddChild(gamblebutton_);
	gamblebutton_->SetPosition(IntVector2(graphics->GetWidth()-gamblebutton_->GetWidth(), 0));
	SubscribeToEvent(gamblebutton_, StringHash("Pressed"), URHO3D_HANDLER(LifeBubbleUI, HandleGamble));
}

void LifeBubbleUI::HandleGamble(StringHash eventType, VariantMap &eventData)
{
	auto pd=GetSubsystem<PlayerData>();
	double energy=pd->GetEnergy();
	if(energy<400.0) return;
	pd->SetEnergy(energy-400.0);

	int rr=roll(1,100);
	ItemRarity ir=IRNormal;
	if(rr<4) ir=IRUnique;
	else if(rr<14) ir=IRRare;
	else if(rr<64) ir=IRMagic;

	int rt=roll(1,100);
	EquipmentSlots slot=EqBlade;
	if(rt<33) slot=EqTurret;
	else if(rt<66) slot=EqShell;

	auto item=pd->GenerateRandomEquipmentItem(slot, ir, 10);
	if(item) pd->DropItem(item, node_->GetPosition(), node_->GetPosition()+Vector3(rollf(-5.0,5.0), 0, rollf(-5.0,5.0)));
}

void LifeBubbleUI::Update(float dt)
{
	auto vitals=node_->GetComponent<PlayerVitals>();
	if(vitals)
	{
		float cl=(float)vitals->GetCurrentLife() / (float)vitals->GetMaximumLife();
		healthmat_->SetShaderParameter("Level", Variant(cl));
		Text *health=dynamic_cast<Text*>(element_->GetChild("Health", true));
		health->SetText(String((int)vitals->GetCurrentLife()) + "/" + String((int)vitals->GetMaximumLife()));
		Color col = Color(1.0,0.5,0.5).Lerp(Color(0.5,0.5,1.0), cl);
		health->SetColor(col);
	}

	auto pd=GetSubsystem<PlayerData>();
	energyelement_->SetText(String("Energy: ") + String((int)pd->GetEnergy()));

	auto input=GetSubsystem<Input>();
	if(input->GetKeyPress(KEY_Q)) pd->SetAttack(PASpinAttack);
	else if(input->GetKeyPress(KEY_W)) pd->SetAttack(PALaserBeam);

	if(input->GetKeyPress(KEY_I))
	{
		if(pd->IsInventoryScreenVisible()) pd->ShowInventoryScreen(false);
		else pd->ShowInventoryScreen(true);
	}
}

void LifeBubbleUI::Stop()
{
/*
SharedPtr<Scene> rttscene_;
	SharedPtr<Node> rttcameranode_;
	SharedPtr<Viewport> rttviewport_;
	SharedPtr<Material> rttmaterial_;
	SharedPtr<Texture2D> rttrendertexture_;
	Material *healthmat_;

	SharedPtr<Scene> overlayscene_;
	SharedPtr<Node> overlaynode_;
	SharedPtr<Viewport> overlayviewport_;

	SharedPtr<UIElement> element_;
	*/
	//rttscene_->Remove();
	element_->Remove();

	energyelement_->Remove();
	help_->Remove();
	levelelement_->Remove();
	gamblebutton_->Remove();
	Log::Write(LOG_INFO, "Stopping bubble");
}
