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

#include "maze.h"

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
    SetWindowTitleAndIcon();
    CreateConsoleAndDebugHud();
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Game, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Game, HandleKeyUp));
	SubscribeToEvent(StringHash("PostRenderUpdate"), URHO3D_HANDLER(Game, HandlePostRenderUpdate));
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(Game, HandleUpdate));

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI *ui=GetSubsystem<UI>();

	Cursor* cursor=new Cursor(context_);
	cursor->DefineShape(CS_NORMAL, cache->GetResource<Image>("UI/buttons.png"), IntRect(950,50,1000,100), IntVector2(20,15));
	ui->SetCursor(cursor);
	cursor->SetVisible(true);
	cursor->SetPosition(ui->GetRoot()->GetWidth()/2, ui->GetRoot()->GetHeight()/2);


	// Testing
	StatSet stats;
	StringHasherType hasher;

	stats.LoadJSON(cache->GetResource<JSONFile>("Objects/teststat.json")->GetRoot());
	StatSetCollection sc;
	sc.push_back(&stats);

	String st=String("TestStat1: ") + String(GetStatValue(sc, "TestStat1"));
	Log::Write(LOG_INFO, st);
	st=String("TestStat2: ") + String(GetStatValue(sc, "TestStat2"));
	Log::Write(LOG_INFO, st);

	MazeGenerator maze;

	maze.init(8,8);
	maze.setAllEdges();
	maze.generateDepthFirstMaze(0,0);

	for(unsigned int x=0; x<maze.getCellWidth(); ++x)
	{
		String row;
		for(unsigned int y=0; y<maze.getCellHeight(); ++y)
		{
			row = row + String((int)maze.getEdgePattern(x,y)) + String(" ");
		}
		Log::Write(LOG_INFO, row);
	}

	scene_=new Scene(context_);
	scene_->CreateComponent<Octree>();
	auto dbg=scene_->CreateComponent<DebugRenderer>();
	auto nav=scene_->CreateComponent<DynamicNavigationMesh>();
	scene_->CreateComponent<Navigable>();
	scene_->CreateComponent<CrowdManager>();

	nav->SetAgentHeight(1.0);
	nav->SetAgentRadius(1.0f);
	nav->SetAgentMaxClimb(0.01);
	nav->SetCellSize(1.0);
	nav->SetCellHeight(0.5);
	nav->SetTileSize(64);

	LoadLightingAndCamera(scene_, String("Areas/Test"));


	for(unsigned int x=0; x<maze.getCellWidth(); ++x)
	{
		for(unsigned int y=0; y<maze.getCellHeight(); ++y)
		{
			unsigned int p=maze.getEdgePattern(x,y);
			auto nd=scene_->CreateChild();
			int rl=roll(0,100);
			String type;
			if(rl<=33) type="B";
			else type="A";

			auto md=nd->CreateComponent<StaticModel>();
			md->SetModel(cache->GetResource<Model>(String("Areas/Test/Models/Floor") + String(p) + String("_") + type + String(".mdl")));
			//md->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
			md->SetMaterial(cache->GetResource<Material>("Areas/Test/floormaterial.xml"));
			md->SetCastShadows(true);

			md=nd->CreateComponent<StaticModel>();
			md->SetModel(cache->GetResource<Model>(String("Areas/Test/Models/Wall") + String(p) + String("_") + type + String(".mdl")));
			md->SetMaterial(cache->GetResource<Material>("Areas/Test/wallmaterial.xml"));
			md->SetCastShadows(true);

			nd->SetPosition(Vector3(y*100, 0, x*100));
			//nd->SetScale(Vector3(0.1,0.1,0.1));
		}
	}

	nav->Build();

	/*test_=scene_->CreateChild();
	{
	auto md=test_->CreateComponent<StaticModel>();
	md->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	md->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
	md->SetCastShadows(true);
	}
*/

	/*Node *n_=scene_->CreateChild("Dude");
	{
	auto ca=n_->CreateComponent<CrowdAgent>();
	ca->SetRadius(1.0);
	ca->SetHeight(2.0);
	ca->SetMaxSpeed(30.0);
	ca->SetMaxAccel(400.0);
	ca->SetNavigationQuality(NAVIGATIONQUALITY_HIGH);
	ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);


	auto md=n_->CreateComponent<AnimatedModel>();
	md->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Body.mdl"));
	md->SetMaterial(cache->GetResource<Material>("Objects/DungeonBot3000/Materials/drivewheel.xml"));
	md->SetCastShadows(true);
	md=n_->CreateComponent<AnimatedModel>();
	md->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Carriage.mdl"));
	md->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	md->SetCastShadows(true);
	md=n_->CreateComponent<AnimatedModel>();
	md->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Turret.mdl"));
	md->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	md->SetCastShadows(true);

	auto rb=md->GetSkeleton().GetBone("LBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}

	rb=md->GetSkeleton().GetBone("RBlade");
	if(rb)
	{
		Node *bl=rb->node_->CreateChild();
		auto smd=bl->CreateComponent<StaticModel>();
		smd->SetModel(cache->GetResource<Model>("Objects/DungeonBot3000/Models/Blade.mdl"));
		smd->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
	}

		auto cc=n_->CreateComponent<CombatController>();
		if(cc) cc->SetObjectPath("Objects/DungeonBot3000");
	}*/
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



	//BoundingBox bbox=scene_->GetBoundingBox();
	for(unsigned int i=0; i<40; ++i)
	{
		Node *en=scene_->CreateChild();
		auto ca=en->CreateComponent<CrowdAgent>();
		ca->SetRadius(1.0);
		ca->SetHeight(2.0);
		ca->SetMaxSpeed(rollf(20.0f, 30.0f));
		ca->SetMaxAccel(400.0);
		ca->SetNavigationQuality(NAVIGATIONQUALITY_HIGH);
		ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);

		auto md=en->CreateComponent<AnimatedModel>();
		md->SetModel(cache->GetResource<Model>("Objects/Dude/Models/dude-base.mdl"));
		md->SetMaterial(cache->GetResource<Material>("Objects/Dude/Materials/skin.xml"));
		md->SetCastShadows(true);
		md=en->CreateComponent<AnimatedModel>();
		md->SetModel(cache->GetResource<Model>("Objects/Dude/Models/dude-male_casualsuit04.mdl"));
		md->SetMaterial(cache->GetResource<Material>("Objects/Dude/Materials/clothes.xml"));
		md->SetCastShadows(true);

		float x=rollf(0.0f,1000.0f);
		float z=rollf(0.0f,1000.0f);

		Vector3 pos=nav->MoveAlongSurface(Vector3(0,0,0), Vector3(x,0,z));
		pos.y_=0;
		en->SetPosition(pos);

		dudes_.push_back(en);
		auto ac=en->CreateComponent<AnimationController>();
		ac->Play("Objects/DungeonBot3000/Models/Walk.ani", 0, true, 0.0f);

		auto cc=en->CreateComponent<CombatController>();
		if(cc) cc->SetObjectPath("Objects/DungeonBot3000");
	}

	auto ac=n_->CreateComponent<AnimationController>();
	ac->Play("Objects/DungeonBot3000/Models/Walk.ani", 0, true, 0.0f);

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
	console->Toggle();

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

	auto input=GetSubsystem<Input>();
	auto cam=scene_->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
	/*IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
	Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);
	if(input->GetMouseButtonDown(MOUSEB_LEFT) )
	{
		auto ca=scene_->GetChild("Dude")->GetComponent<CrowdAgent>();
		ca->SetTargetPosition(Vector3(ground.x_,0,ground.y_));
		//Log::Write(LOG_INFO, String("Mousepos:") + String(mousepos.x_) + "," + String(mousepos.y_));
		//Log::Write(LOG_INFO, String("Groundpos:") + String(ground.x_) + "," + String(ground.y_));

	}
	else
	{
		auto dd=scene_->GetChild("Dude");
		auto ca=dd->GetComponent<CrowdAgent>();
		//ca->SetTargetPosition(dd->GetPosition());
	}*/

	for(auto i : dudes_)
	{
		auto *ca=i->GetComponent<CrowdAgent>();
		ca->SetTargetPosition(scene_->GetChild("Dude")->GetPosition()+Vector3(rollf(-8.0f,8.0f),0,rollf(-8.0f,8.0f)));
	}

	VariantMap vm;
	vm[position]=scene_->GetChild("Dude")->GetPosition();
	SendEvent(CameraSetPosition, vm);
	//else Log::Write(LOG_INFO, "No pick");
}

URHO3D_DEFINE_APPLICATION_MAIN(Game)
