#include "lightingcamera.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/LuaScript/LuaFile.h>
#include <Urho3D/LuaScript/LuaFunction.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Zone.h>

#include "Components/thirdpersoncamera.h"

bool LoadLightingAndCamera(Scene *scene, String levelpath)
{
	ResourceCache *cache=scene->GetContext()->GetSubsystem<ResourceCache>();
	if(!cache) return false;

	Node *lightnode=scene->CreateChild();

	JSONFile *lighting=cache->GetResource<JSONFile>(levelpath+"/lighting.json");
	if(!lighting)
	{
		Log::Write(LOG_DEBUG, String("Could not load lighting for level ")+levelpath);
		return false;
	}

	const JSONValue &lightingInfo=lighting->GetRoot();
	if(!lightingInfo.IsObject())
	{
		Log::Write(LOG_DEBUG, String("Malformed lighting info: ")+levelpath);
		return false;
	}

	const JSONObject &lightingObject=lightingInfo.GetObject();

	if(lightnode)
	{
		// Main light
		Node *childnode=lightnode->CreateChild();
		if(childnode)
		{
			childnode->SetDirection(Vector3(1.5f,-1.0f,0.1f));
			const JSONArray &mainInfo=lightingObject["main"]->GetArray();
			if(mainInfo.Size()<3)
			{
				Log::Write(LOG_DEBUG, String("Malformed main light record in ")+levelpath);
				return false;
			}

			Light *light=childnode->CreateComponent<Light>();
			if(light)
			{
				light->SetLightType(LIGHT_DIRECTIONAL);
				light->SetColor(Color(mainInfo[0].GetFloat(), mainInfo[1].GetFloat(), mainInfo[2].GetFloat()));
				light->SetCastShadows(true);
				light->SetSpecularIntensity(0.1f);
			}
		}

		// Back light
		childnode=lightnode->CreateChild();
		if(childnode)
		{
			childnode->SetDirection(Vector3(-1.5f,-1.0f,-0.1f));
			const JSONArray &backInfo=lightingObject["back"]->GetArray();
			if(backInfo.Size()<3)
			{
				Log::Write(LOG_DEBUG, String("Malformed back light record in ")+levelpath);
				return false;
			}

			Light *light=childnode->CreateComponent<Light>();
			if(light)
			{
				light->SetLightType(LIGHT_DIRECTIONAL);
				light->SetColor(Color(backInfo[0].GetFloat(), backInfo[1].GetFloat(), backInfo[2].GetFloat()));
				light->SetCastShadows(false);
				light->SetSpecularIntensity(0.01f);
			}
		}

		// Zone

		Zone *zone=lightnode->CreateComponent<Zone>();
		if(zone)
		{
			const JSONArray &fogcolor=lightingObject["fog"]->GetArray();
			if(fogcolor.Size()<3)
			{
				Log::Write(LOG_DEBUG, String("Malformed fog record in ")+levelpath);
				return false;
			}
			zone->SetFogColor(Color(fogcolor[0].GetFloat(), fogcolor[1].GetFloat(), fogcolor[2].GetFloat()));

			const JSONArray &ambcolor=lightingObject["ambient"]->GetArray();
			if(ambcolor.Size()<3)
			{
				Log::Write(LOG_DEBUG, String("Malformed ambient color record in ")+levelpath);
				return false;
			}
			zone->SetAmbientColor(Color(ambcolor[0].GetFloat(), ambcolor[1].GetFloat(), ambcolor[2].GetFloat()));

			zone->SetFogStart(lightingObject["near"]->GetFloat());
			zone->SetFogEnd(lightingObject["far"]->GetFloat());
			zone->SetBoundingBox(BoundingBox(-1000,1000));
		}

	}

	Log::Write(LOG_DEBUG, String("Create camera"));
	Node *camnode=scene->CreateChild("Camera");
	ThirdPersonCamera *cam=camnode->CreateComponent<ThirdPersonCamera>();

	cam->SetClipCamera(false);
	cam->SetSpringTrack(false);
	cam->SetClipDist(lightingObject["far"]->GetFloat());
	cam->SetCellSize(128);
	cam->SetOrthographic(false);
	cam->SetAllowSpin(false);
	cam->SetAllowPitch(true);
	cam->SetRotAngle(0);
	cam->SetCamAngle(40);
	cam->SetMinFollow(2);
	cam->SetMaxFollow(30);
	//cam->SetMinFollowAngle(15);
	//cam->SetMaxFollowAngle(99);

	Log::Write(LOG_DEBUG, String("Setup viewport and post-process"));
	Viewport *vp=cam->GetViewport();
	SharedPtr<RenderPath> rp=vp->GetRenderPath()->Clone();
	rp->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
	vp->SetRenderPath(rp);
	return true;
}
