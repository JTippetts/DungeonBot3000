#include "testlevelstate.h"
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


#include "../lightingcamera.h"
#include "../maze2.h"
#include "../Components/scenefader.h"
#include "../playerdata.h"
#include "../Components/hoverhandler.h"
#include "../Components/levelchanger.h"
#include "../Components/vitals.h"
#include "../Components/mainmenu.h"

#include "../scenetools.h"

int roll(int low, int high);
float rollf(float, float);

TestLevelState::TestLevelState(Context *context) : GameStateBase(context)
{
}

void TestLevelState::Start()
{
	auto cache=context_->GetSubsystem<ResourceCache>();

	unsigned int level=data_[StringHash("level")].GetUInt();
	unsigned int previouslevel=data_[StringHash("previouslevel")].GetUInt();
	String levelpath="Areas/Test";

	unsigned int w=3,h=3;
	float size=200.0f;

	scene_=SharedPtr<Scene>(new Scene(context_));
	scene_->CreateComponent<Octree>();
	auto dbg=scene_->CreateComponent<DebugRenderer>();
	auto nav=scene_->CreateComponent<DynamicNavigationMesh>();
	scene_->CreateComponent<Navigable>();
	scene_->CreateComponent<CrowdManager>();
	scene_->CreateComponent<HoverHandler>();
	auto musicsource=scene_->CreateComponent<SoundSource>();
	musicsource->SetSoundType(SOUND_MUSIC);
	auto music=cache->GetResource<Sound>("Music/Gravity Sound - Chase CC BY 4.0_0.ogg");
	if(music)
	{
		music->SetLooped(true);
		musicsource->Play(music);
	}
	auto audio=context_->GetSubsystem<Audio>();
	if(audio)
	{
		audio->SetMasterGain(SOUND_MUSIC, 0.25);
	}


	nav->SetAgentHeight(1.0);
	nav->SetAgentRadius(2.0f);
	nav->SetAgentMaxClimb(0.01);
	nav->SetCellSize(1.0);
	nav->SetCellHeight(0.5);
	nav->SetTileSize(64);

	LoadLightingAndCamera(scene_, levelpath);

	Maze2 maze;
	maze.Init(w,h);
	maze.DepthFirstMaze(0,0);

	unsigned int upx=roll(0,w-1);
	unsigned int upy=roll(0,h-1);

	unsigned int downx,downy;
	do
	{
		downx=roll(0,w-1);
		downy=roll(0,h-1);
	} while(downx==upx && downy==upy);

	unsigned int bossx, bossy;
	do
	{
		bossx=roll(0,w-1);
		bossy=roll(0,h-1);
	} while((bossx==downx && bossy==downy) || (bossx==upx && bossy==upy));

	for(unsigned int x=0; x<maze.GetCellWidth(); ++x)
	{
		for(unsigned int y=0; y<maze.GetCellHeight(); ++y)
		{
			unsigned int p=maze.GetCellPattern(x,y);
			int rl=roll(0,100);

			String path;
			if(rl<=50)
			{
				if(x==downx && y==downy && level!=10) path = levelpath + String("/tiledown") + String(p) + "_A.json";
				else path = levelpath + String("/tile") + String(p) + "_A.json";
			}
			else
			{
				if(x==downx && y==downy && level!=10) path = levelpath + String("/tiledown") + String(p) + "_B.json";
				else path = levelpath + String("/tile") + String(p) + "_B.json";
			}

			auto f=cache->GetResource<JSONFile>(path);
			if(f)
			{
				auto nd=scene_->InstantiateJSON(f->GetRoot(), Vector3(y*size + size*0.5f, 0.0f, x*size + size*0.5f), Quaternion());
				nd->SetScale(Vector3(size/200.0f, size/200.0f, size/200.0f));

				if(x==upx && y==upy && level!=1)
				{
					auto up=SpawnObject(scene_, levelpath+"/stairsup.xml", nd->GetWorldPosition());
					auto changer=up->GetComponent<LevelChanger>();
					changer->SetDestination(level-1);
				}
				else if(x==downx && y==downy && level != 10)
				{
					auto dn=SpawnObject(scene_, levelpath+"/stairsdown.xml", nd->GetWorldPosition());
					auto changer=dn->GetComponent<LevelChanger>();
					changer->SetDestination(level+1);
				}
			}
		}
	}

	nav->Build();

	for(unsigned int x=0; x<maze.GetCellWidth(); ++x)
	{
		for(unsigned int y=0; y<maze.GetCellHeight(); ++y)
		{
			if (!(x==downx && y==downy) && !(x==upx && y==upy))
			{
				// Spawn a group of mobs
				Vector3 roomcenter(y*size+size*0.5f, 0, x*size+size*0.5f);
				Vector3 extents(50,0,50);
				int rl;
				rl=roll(1,100);
				if(x==bossx && y==bossy)
				{
					if(level==10)
					{
						// Spawn KHAwk and jbadams
						Vector3 pt=nav->FindNearestPoint(roomcenter+Vector3(rollf(-size*0.5f, size*0.5f), 0.0f, rollf(-size*0.5f, size*0.5f)), extents);
						pt=nav->MoveAlongSurface(roomcenter, pt);
						auto n=SpawnObject(scene_, "Objects/Mobs/KHawk/object.xml", pt);
						if(n)
						{
							n->GetComponent<EnemyVitals>()->SetLevel(level);
							n->SetVar("hoverable", true);
						}
						pt=nav->FindNearestPoint(roomcenter+Vector3(rollf(-size*0.5f, size*0.5f), 0.0f, rollf(-size*0.5f, size*0.5f)), extents);
						pt=nav->MoveAlongSurface(roomcenter, pt);
						n=SpawnObject(scene_, "Objects/Mobs/jbadams/object.xml", pt);
						if(n)
						{
							n->GetComponent<EnemyVitals>()->SetLevel(level);
							n->SetVar("hoverable", true);
						}
					}
					else
					{
						// Spawn an emeritus
						Vector3 pt=nav->FindNearestPoint(roomcenter+Vector3(rollf(-size*0.5f, size*0.5f), 0.0f, rollf(-size*0.5f, size*0.5f)), extents);
						pt=nav->MoveAlongSurface(roomcenter, pt);
						auto n=SpawnObject(scene_, "Objects/Mobs/Emeritus/object.xml", pt);
						if(n)
						{
							n->GetComponent<EnemyVitals>()->SetLevel(level);
							n->SetVar("hoverable", true);
						}
					}
				}

				if(rl < 50)
				{
					// Spawn some moderators
					for(int nm = 0; nm<roll(1,3); ++nm)
					{
						Vector3 pt=nav->FindNearestPoint(roomcenter+Vector3(rollf(-size*0.5f, size*0.5f), 0.0f, rollf(-size*0.5f, size*0.5f)), extents);
						pt=nav->MoveAlongSurface(roomcenter, pt);
						auto n=SpawnObject(scene_, "Objects/Mobs/Moderator/object.xml", pt);
						if(n)
						{
							n->GetComponent<EnemyVitals>()->SetLevel(level);
							n->SetVar("hoverable", true);
						}
					}
				}

				int numusers=roll(6*level,8*level);
				for(int c=0; c<numusers; ++c)
				{
					Vector3 pt=nav->FindNearestPoint(roomcenter+Vector3(rollf(-size*0.5f, size*0.5f), 0.0f, rollf(-size*0.5f, size*0.5f)), extents);
					pt=nav->MoveAlongSurface(roomcenter, pt);
					auto n=SpawnObject(scene_, "Objects/Mobs/User/object.xml", pt);
					if(n)
					{
						n->GetComponent<EnemyVitals>()->SetLevel(level);
						n->SetVar("hoverable", true);
					}
				}

			}
		}
	}

	auto pd=scene_->GetSubsystem<PlayerData>();
	if(pd)
	{
		if(level < previouslevel)
		{
			// Spawn at down stairs
			pd->SpawnPlayer(scene_, Vector3((float)downy * size + (size*0.5f+10.0f), 0.0f, (float)downx * size + size*0.5f));
		}
		else if(level > previouslevel)
		{
			pd->SpawnPlayer(scene_, Vector3((float)upy * size + (size*0.5f+10.0f), 0.0f, (float)upx * size + size*0.5f));
		}
		else
		{
			pd->SpawnPlayer(scene_, Vector3((float)upy * size + (size*0.5f+10.0f), 0.0f, (float)upx * size + size*0.5f));
		}
	}
}
