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

#include "vitals.h"

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

	rttscene_ = new Scene(context_);
	rttscene_->CreateComponent<Octree>();

	rttcameranode_ = rttscene_->CreateChild("Camera");
    auto camera = rttcameranode_->CreateComponent<Camera>();

	rttcameranode_->SetPosition(Vector3(0.0, 5.0, 0.0));

	camera->SetOrthographic(true);
	camera->SetOrthoSize(100);
	rttcameranode_->LookAt(Vector3(0,0,0), Vector3(0,1,0));

	auto planeNode = rttscene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(100.0, 1.0, 100.0));
    auto planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
	healthmat_=cache->GetResource<Material>("Materials/health.xml");
    planeObject->SetMaterial(healthmat_);
	healthmat_->SetShaderParameter("Level", Variant(1.0));

	rttrendertexture_ = new Texture2D(context_);
    rttrendertexture_->SetSize(128, 128, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
    rttrendertexture_->SetFilterMode(FILTER_BILINEAR);

    rttmaterial_ = new Material(context_);
    rttmaterial_->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffUnlit.xml"));
    rttmaterial_->SetTexture(TU_DIFFUSE, rttrendertexture_);
    RenderSurface* surface = rttrendertexture_->GetRenderSurface();
    rttviewport_ = new Viewport(context_, rttscene_, camera);
    surface->SetViewport(0, rttviewport_);
	surface->SetUpdateMode(SURFACE_UPDATEALWAYS);

	/*
	uisprite_ = new Sprite(context_);
	uisprite_->SetTexture(rttrendertexture_);
	uisprite_->SetSize(IntVector2(128,128));
	uisprite_->SetFullImageRect();

	auto ui=GetSubsystem<UI>();
	ui->GetRoot()->AddChild(uisprite_);
	uisprite_->SetVisible(true);*/
	auto ui=GetSubsystem<UI>();
	auto graphics=GetSubsystem<Graphics>();
	element_ = ui->LoadLayout(cache->GetResource<XMLFile>("UI/healthbubble.xml"));
	auto sprite = dynamic_cast<Sprite *>(element_->GetChild("Bubble", true));
	if(sprite)
	{
		sprite->SetTexture(rttrendertexture_);
		sprite->SetFullImageRect();
	}
	ui->GetRoot()->AddChild(element_);
	element_->SetPosition(IntVector2(-38, graphics->GetHeight()-256+38));
}

void LifeBubbleUI::Update(float dt)
{
	auto vitals=node_->GetComponent<PlayerVitals>();
	if(vitals)
	{
		float cl=(float)vitals->GetCurrentLife() / (float)vitals->GetMaximumLife();

		healthmat_->SetShaderParameter("Level", Variant(cl));
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
	Log::Write(LOG_INFO, "Stopping bubble");
}
