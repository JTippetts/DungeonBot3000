#include "game.h"

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
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>

#include "stats.h"
#include "RegisterComponents.h"

#include "lightingcamera.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "Components/vitals.h"

#include "playerdata.h"
#include "itemnametagcontainer.h"
#include "playeractionstates.h"
#include "enemyactionstates.h"

#include "maze2.h"
#include "combat.h"

float roll(int low, int high)
{
	static std::mt19937 gen;
	static bool first=true;
	std::uniform_int_distribution<int> dist(low,high);

	if(first)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		gen.seed(seed);
		first=false;
	}
	return dist(gen);
}

float rollf(float low, float high)
{
	static std::mt19937 gen;
	static bool first=true;
	std::uniform_real_distribution<float> dist(low,high);

	if(first)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		gen.seed(seed);
		first=false;
	}
	return dist(gen);
}

double rolld(double low, double high)
{
	static std::mt19937 gen;
	static bool first=true;
	std::uniform_real_distribution<double> dist(low,high);

	if(first)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		gen.seed(seed);
		first=false;
	}
	return dist(gen);
}

Game::Game(Context* context) :
    Application(context), x(0), z(0)
{
}

void Game::Setup()
{
    // Modify engine startup parameters
    engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
    engineParameters_[EP_LOG_NAME]     = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
    engineParameters_[EP_FULL_SCREEN]  = false;
    engineParameters_[EP_HEADLESS]     = false;
    engineParameters_[EP_SOUND]        = false;

    // Construct a search path to find the resource prefix with two entries:
    // The first entry is an empty path which will be substituted with program/bin directory -- this entry is for binary when it is still in build tree
    // The second and third entries are possible relative paths from the installed program/bin directory to the asset directory -- these entries are for binary when it is in the Urho3D SDK installation location
    if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS))
        engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../share/Resources;../share/Urho3D/Resources";


	RegisterCustomComponents(context_);
}

void Game::Start()
{
	//MazeGenerator maze(8,8);
	Maze2 maze;
	maze.Init(8,8);

    SetWindowTitleAndIcon();
    CreateConsoleAndDebugHud();
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Game, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Game, HandleKeyUp));
	SubscribeToEvent(StringHash("PostRenderUpdate"), URHO3D_HANDLER(Game, HandlePostRenderUpdate));
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(Game, HandleUpdate));

	context_->RegisterSubsystem(new PlayerData(context_));
	context_->RegisterSubsystem(new ItemNameTagContainer(context_));

	auto pd=context_->GetSubsystem<PlayerData>();
	if(pd)
	{
		pd->LoadItemModTable("Tables/Items/itemmods.json");
		pd->LoadItemModTiers("Tables/Items/itemmodtiers.json");
		pd->LoadBaseStats("Tables/Player/base.json");
		pd->LoadSkillStats("Tables/Skills/skillstats.json");
	}

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI *ui=GetSubsystem<UI>();

	Cursor* cursor=new Cursor(context_);
	cursor->DefineShape(CS_NORMAL, cache->GetResource<Image>("UI/buttons.png"), IntRect(950,50,1000,100), IntVector2(20,15));
	ui->SetCursor(cursor);
	cursor->SetVisible(true);
	cursor->SetPosition(ui->GetRoot()->GetWidth()/2, ui->GetRoot()->GetHeight()/2);

	//maze.setAllEdges();
	maze.DepthFirstMaze(0,0);

	for(unsigned int x=0; x<maze.GetCellWidth(); ++x)
	{
		String row;
		for(unsigned int y=0; y<maze.GetCellHeight(); ++y)
		{
			row = row + String((int)maze.GetCellPattern(x,y)) + String(" ");
		}
		Log::Write(LOG_INFO, row);
	}

	scene_=new Scene(context_);
	scene_->CreateComponent<Octree>();
	auto dbg=scene_->CreateComponent<DebugRenderer>();
	auto nav=scene_->CreateComponent<DynamicNavigationMesh>();
	scene_->CreateComponent<Navigable>();
	scene_->CreateComponent<CrowdManager>();

	pd->SetCurrentScene(scene_);

	nav->SetAgentHeight(1.0);
	nav->SetAgentRadius(2.0f);
	nav->SetAgentMaxClimb(0.01);
	nav->SetCellSize(1.0);
	nav->SetCellHeight(0.5);
	nav->SetTileSize(64);

	LoadLightingAndCamera(scene_, String("Areas/Test"));


	for(unsigned int x=0; x<maze.GetCellWidth(); ++x)
	{
		for(unsigned int y=0; y<maze.GetCellHeight(); ++y)
		{
			unsigned int p=maze.GetCellPattern(x,y);
			auto nd=scene_->CreateChild();
			int rl=roll(0,100);

			String path;
			if(rl<=33)
			{
				path = String("Areas/Test/tile") + String(p) + "_A.json";
			}
			else
			{
				path = String("Areas/Test/tile") + String(p) + "_B.json";
			}

			auto f=cache->GetResource<JSONFile>(path);
			if(f)
			{
				auto nd=scene_->InstantiateJSON(f->GetRoot(), Vector3(y*200.0f + 100.0f, 0.0f, x*200.0f + 100.0f), Quaternion());
			}

			/*auto md=nd->CreateComponent<StaticModel>();
			md->SetModel(cache->GetResource<Model>(String("Areas/Test/Models/Floor") + String(p) + String("_") + type + String(".mdl")));
			//md->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
			md->SetMaterial(cache->GetResource<Material>("Areas/Test/floormaterial.xml"));
			md->SetCastShadows(true);

			md=nd->CreateComponent<StaticModel>();
			md->SetModel(cache->GetResource<Model>(String("Areas/Test/Models/Wall") + String(p) + String("_") + type + String(".mdl")));
			md->SetMaterial(cache->GetResource<Material>("Areas/Test/wallmaterial.xml"));
			md->SetCastShadows(true);

			nd->SetPosition(Vector3(y*200+100, 0, x*200+100));
			nd->SetScale(Vector3(1,1,1));

			nd->SetVar("world", true);

			auto stairs=nd->CreateChild();
			md=stairs->CreateComponent<StaticModel>();
			md->SetModel(cache->GetResource<Model>(String("Areas/Test/Models/StairsUp.mdl")));
			md->SetMaterial(cache->GetResource<Material>("Areas/Test/wallmaterial.xml"));
			md->SetCastShadows(true);
			//nd->SetScale(Vector3(0.1,0.1,0.1));*/
		}
	}

	nav->Build();


	XMLFile *file=cache->GetResource<XMLFile>("Objects/DungeonBot3000/object.xml");
	Node *n_=scene_->InstantiateXML(file->GetRoot(), Vector3(0,0,0), Quaternion(0,Vector3(0,1,0)));
	auto rb=n_->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("LBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}

	rb=n_->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("RBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}

	// Give DB3000 a starter blade
	pd->EquipItem(EquipmentItemDef(EqBlade, IRNormal, "Starter Blade", "", "", {"StarterBladeImplicit"}), false);
	pd->DropItem(EquipmentItemDef(EqBlade, IRMagic, "Steel Blade", "", "", {"SteelBladeImplicit", "Invigorating", "InfernalBladeBurnImplicit", "Bloodsucking"}), Vector3(100,0,100), Vector3(102,0,102));
	pd->SetPlayerNode(n_);
	n_->SetPosition(Vector3(100,0,100));

	for(unsigned int i=0; i<300; ++i)
	{

		Vector3 pos;
		do
		{
			pos=nav->GetRandomPoint(); //nav->MoveAlongSurface(Vector3(0,0,0), Vector3(x,0,z));
		} while(pos.y_ > 0.5);
		//Log::Write(LOG_INFO, String(pos.y_));
		pos.y_=0;
		XMLFile *xfile;
		auto rl=rollf(0,100);
		//if(rl < 30) xfile=cache->GetResource<XMLFile>("Objects/Mobs/jbadams/object.xml");
		Node *n;
		if(rl < 5)
		{
			xfile=cache->GetResource<XMLFile>("Objects/Mobs/KHawk/object.xml");
			n=scene_->InstantiateXML(xfile->GetRoot(), pos, Quaternion(0,Vector3(0,1,0)));
			auto rb=n->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("weapon_r");
			if(rb)
			{
				Node *bl=rb->node_->CreateChild();
				auto smd=bl->CreateComponent<StaticModel>();
				//smd->SetModel(cache->GetResource<Model>("Objects/Mobs/jbadams/Models/Hammer.mdl"));
				smd->SetModel(cache->GetResource<Model>("Objects/Mobs/KHawk/Models/RocketLauncher.mdl"));
				smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
			}
		}
		else if(rl < 40)
		{
			xfile=cache->GetResource<XMLFile>("Objects/Mobs/Moderator/object.xml");
			n=scene_->InstantiateXML(xfile->GetRoot(), pos, Quaternion(0,Vector3(0,1,0)));
		}
		else if(rl < 45)
		{
			xfile=cache->GetResource<XMLFile>("Objects/Mobs/jbadams/object.xml");
			n=scene_->InstantiateXML(xfile->GetRoot(), pos, Quaternion(0,Vector3(0,1,0)));
			auto rb=n->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("weapon_r");
			if(rb)
			{
				Node *bl=rb->node_->CreateChild();
				auto smd=bl->CreateComponent<StaticModel>();
				smd->SetModel(cache->GetResource<Model>("Objects/Mobs/jbadams/Models/Hammer.mdl"));
				smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
			}
		}
		else
		{
			xfile=cache->GetResource<XMLFile>("Objects/Mobs/User/object.xml");
			n=scene_->InstantiateXML(xfile->GetRoot(), pos, Quaternion(0,Vector3(0,1,0)));
		}


		auto vtls=n->GetComponent<EnemyVitals>();
		if(vtls)
		{
			vtls->SetLevel(10);
		}
	}

}

void Game::Stop()
{
    engine_->DumpResources(true);
}

void Game::SetWindowTitleAndIcon()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Image* icon = cache->GetResource<Image>("Textures/UrhoIcon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("DungeonBot3000");
}

void Game::CreateConsoleAndDebugHud()
{
    // Get default style
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.8f);
	//console->Toggle();

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);
}

void Game::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyUp;

    int key = eventData[P_KEY].GetInt();

    // Close console (if open) or exit when ESC is pressed
    if (key == KEY_ESCAPE)
    {
        Console* console = GetSubsystem<Console>();
        if (console->IsVisible())
            console->SetVisible(false);
        else
        {
           engine_->Exit();
        }
    }
}

void Game::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();
    if (key == KEY_F1)
        GetSubsystem<Console>()->Toggle();

    else if (key == KEY_F2)
        GetSubsystem<DebugHud>()->ToggleAll();

    else if (key == KEY_PRINTSCREEN)
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        Image screenshot(context_);
        graphics->TakeScreenShot(screenshot);
        screenshot.SavePNG(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
            Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png");
    }
}

void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
	//scene_->GetComponent<DynamicNavigationMesh>()->DrawDebugGeometry(true);
	//scene_->GetComponent<CrowdManager>()->DrawDebugGeometry(true);

}

void Game::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
	static StringHash TimeStep("TimeStep"), CameraSetPosition("CameraSetPosition"), position("position");
	float dt=eventData[TimeStep].GetFloat();

	auto ui=GetSubsystem<UI>();
	auto input=GetSubsystem<Input>();

	// Do hover
	IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=ui->GetCursorPosition();

	auto element = ui->GetElementAt(mousepos);
	if(element)
	{
		while(element->GetParent() != ui->GetRoot()) element=element->GetParent();
	}
}

URHO3D_DEFINE_APPLICATION_MAIN(Game)
