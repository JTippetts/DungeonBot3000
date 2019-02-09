#include "scenetools.h"

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>


#include "lightingcamera.h"
#include "maze2.h"
#include "Components/scenefader.h"

using namespace Urho3D;

int roll(int low, int high);

SharedPtr<Scene> CreateLevel(Context *context, String levelpath, unsigned int level)
{
	auto cache=context->GetSubsystem<ResourceCache>();

	SharedPtr<Scene> scene(new Scene(context));
	scene->CreateComponent<Octree>();
	auto dbg=scene->CreateComponent<DebugRenderer>();
	auto nav=scene->CreateComponent<DynamicNavigationMesh>();
	scene->CreateComponent<Navigable>();
	scene->CreateComponent<CrowdManager>();
	auto musicsource=scene->CreateComponent<SoundSource>();
	musicsource->SetSoundType(SOUND_MUSIC);
	auto music=cache->GetResource<Sound>("Music/Gravity Sound - Chase CC BY 4.0_0.ogg");
	if(music)
	{
		music->SetLooped(true);
		musicsource->Play(music);
	}
	auto audio=context->GetSubsystem<Audio>();
	if(audio)
	{
		audio->SetMasterGain(SOUND_MUSIC, 0.5);
	}


	nav->SetAgentHeight(1.0);
	nav->SetAgentRadius(2.0f);
	nav->SetAgentMaxClimb(0.01);
	nav->SetCellSize(1.0);
	nav->SetCellHeight(0.5);
	nav->SetTileSize(64);

	LoadLightingAndCamera(scene, levelpath);

	Maze2 maze;
	maze.Init(8,8);
	maze.DepthFirstMaze(0,0);

	for(unsigned int x=0; x<maze.GetCellWidth(); ++x)
	{
		for(unsigned int y=0; y<maze.GetCellHeight(); ++y)
		{
			unsigned int p=maze.GetCellPattern(x,y);
			int rl=roll(0,100);

			String path;
			if(rl<=50)
			{
				path = levelpath + String("/tile") + String(p) + "_A.json";
			}
			else
			{
				path = levelpath + String("/tile") + String(p) + "_B.json";
			}

			auto f=cache->GetResource<JSONFile>(path);
			if(f)
			{
				auto nd=scene->InstantiateJSON(f->GetRoot(), Vector3(y*200.0f + 100.0f, 0.0f, x*200.0f + 100.0f), Quaternion());
			}
		}
	}

	nav->Build();

	auto fader=scene->CreateComponent<SceneFader>();
	if(fader)
	{
		fader->SetFadeDuration(0.5);
		fader->SetFadeState(SceneFader::FadingIn);
	}
	return scene;
}
