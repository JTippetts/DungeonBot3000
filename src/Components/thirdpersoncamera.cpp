#include "thirdpersoncamera.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Core/Context.h>
#include <cmath>
#include <algorithm>
#include <Urho3D/IO/Log.h>

ThirdPersonCamera::ThirdPersonCamera(Context *context) : LogicComponent(context),
	cellsize_(128), camangle_(30), rotangle_(45), follow_(10), minfollow_(1), maxfollow_(20),
	clipdist_(60), clipcamera_(false), springtrack_(true), allowspin_(true), allowpitch_(true), allowzoom_(true),
	orthographic_(false), curfollow_(10), followvel_(0), pos_(0,0,0), newpos_(0,0,0), posvelocity_(0,0,0), offset_(0.5),
	shakemagnitude_(0), shakespeed_(0), shaketime_(0), shakedamping_(0)
{
	SetUpdateEventMask(USE_UPDATE);
}

void ThirdPersonCamera::RegisterObject(Context *context)
{
	context->RegisterFactory<ThirdPersonCamera>("Camera");
	URHO3D_ACCESSOR_ATTRIBUTE("Cell Size", GetCellSize, SetCellSize, unsigned int, 128, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Azimuth Angle", GetCamAngle, SetCamAngle, float, 30.0f, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Spin Angle", GetRotAngle, SetRotAngle, float, 45.0f, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Clip Distance", GetClipDist, SetClipDist, float, 60.0f, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Spring Tracking", GetSpringTrack, SetSpringTrack, bool, false, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Adjust Azimuth", GetAllowPitch, SetAllowPitch, bool, false, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Adjust Spin", GetAllowSpin, SetAllowSpin, bool, false, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Allow Zoom", GetAllowZoom, SetAllowZoom, bool, true, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Orthographic", GetOrthographic, SetOrthographic, bool, false, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Offset", GetOffset, SetOffset, float, 0.5f, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Min Zoom Distance", GetMinFollow, SetMinFollow, float, 2.0f, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Max Zoom Distance", GetMaxFollow, SetMaxFollow, float, 45.0f, AM_DEFAULT);
}

Vector2 ThirdPersonCamera::WorldToScreen(const Vector3 &pt)
{
	if(camera_) return camera_->WorldToScreenPoint(pt);
	return Vector2(0,0);
}

Vector2 ThirdPersonCamera::WorldToScreenAtLocation(const Vector3 &pt, const Vector3 &location)
{
	Vector3 oldloc=node_->GetWorldPosition();
	node_->SetWorldPosition(location);
	Vector2 pos=WorldToScreen(pt);
	node_->SetWorldPosition(oldloc);
	return pos;
}

void ThirdPersonCamera::Update(float dt)
{
	shaketime_+=dt*shakespeed_;
	float s=std::sin(shaketime_)*shakemagnitude_;
	Vector3 shakepos(std::sin(shaketime_*3.0f)*s, std::cos(shaketime_)*s,0);
	shakemagnitude_-=shakedamping_*dt;
	shakemagnitude_=std::max(shakemagnitude_, 0.0f);
	UI *ui=context_->GetSubsystem<UI>();
	Cursor *cursor=ui->GetCursor();
	Input *input=context_->GetSubsystem<Input>();
	Graphics *graphics=context_->GetSubsystem<Graphics>();

	if(allowzoom_ && !ui->GetElementAt(cursor->GetPosition()))
	{
		float wheel=input->GetMouseMoveWheel();
		follow_-=wheel*dt*400.0f;
		follow_=std::max(minfollow_, std::min(maxfollow_, follow_));
	}

	if(input->GetMouseButtonDown(MOUSEB_MIDDLE) && (allowspin_ || allowpitch_))
	{
		cursor->SetVisible(false);
		if(allowpitch_)
		{
			float mmovey=(float)input->GetMouseMoveY()/(float)graphics->GetHeight();
			camangle_+=mmovey*600.0f;
			camangle_=std::max(1.0f, std::min(camangle_, 89.0f));
			//LOGINFO(String(mmovey));
		}

		if(allowspin_)
		{
			float mmovex=(float)input->GetMouseMoveX()/(float)graphics->GetWidth();
			rotangle_+=mmovex*800.0f;
			while(rotangle_<0) rotangle_+=360.0f;
			while(rotangle_>360) rotangle_-=360.0f;
		}
	}
	else cursor->SetVisible(true);

	SpringFollow(dt);

	if(clipcamera_)
	{
		Ray ray=camera_->GetScreenRay(0.5, 0.5);
		Ray revray=Ray(node_->GetPosition(), ray.direction_*Vector3(-1,-1,-1));
		curfollow_=CameraPick(revray, curfollow_);
	}

	shakenode_->SetPosition(shakepos);

	if(springtrack_)
	{
		SpringPosition(dt);
		node_->SetPosition(pos_);
		//LOGINFO(String(pos_.x_)+","+String(pos_.z_));
	}
	else node_->SetPosition(newpos_);

	//LOGINFO(String(newpos_.x_)+","+String(newpos_.z_));

	node_->SetRotation(Quaternion(rotangle_, Vector3(0,1,0)));
	cameranode_->SetPosition(Vector3(0,0,-curfollow_));
	anglenode_->SetRotation(Quaternion(camangle_, Vector3(1,0,0)));
}

void ThirdPersonCamera::SetFollow(float f)
{
	curfollow_=follow_=(std::max(minfollow_, std::min(maxfollow_, f)));
}

void ThirdPersonCamera::HandleSetCameraPosition(StringHash eventType, VariantMap &eventData)
{
	//Log::Write(LOG_INFO,"Hihihi");
	static StringHash position("position");
	newpos_=eventData[position].GetVector3();
}

void ThirdPersonCamera::HandleResetCameraPosition(StringHash eventType, VariantMap &eventData)
{
	static StringHash position("position");
	pos_=eventData[position].GetVector3();
	newpos_=pos_;
}

void ThirdPersonCamera::HandleRequestCameraRotation(StringHash eventType, VariantMap &eventData)
{
	eventData[StringHash("spin")]=rotangle_;
	eventData[StringHash("pitch")]=camangle_;
}

void ThirdPersonCamera::HandleShakeCamera(StringHash eventType, VariantMap &eventData)
{
	static StringHash magnitude("magnitude"), speed("speed"), damping("damping");
	shakemagnitude_=eventData[magnitude].GetDouble();
	shakespeed_=eventData[speed].GetDouble();
	shakedamping_=eventData[damping].GetDouble();
}

void ThirdPersonCamera::HandleRequestMouseGround(StringHash eventType, VariantMap &eventData)
{
	IntVector2 mousepos;
	Input *input=context_->GetSubsystem<Input>();
	static StringHash location("location");

	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
	Vector3 ground;
	if(PickGround(ground,mousepos.x_,mousepos.y_)) eventData[location]=ground;
	else eventData[location]=Vector2(-1,-1);
	//LOGINFO(String("Pick: ")+String(ground.x_)+String(",")+String(ground.y_));
}

void ThirdPersonCamera::HandleRequestMouseRay(StringHash eventType, VariantMap &eventData)
{
	Ray ray=GetMouseRay();
	static StringHash origin("origin"), direction("direction");
	eventData[origin]=ray.origin_;
	eventData[direction]=ray.direction_;
}

void ThirdPersonCamera::HandleSetCamera(StringHash eventType, VariantMap &eventData)
{
	static StringHash orthographic("orthographic"), rotangle("rotangle"), pitchangle("pitchangle"), allowspin("allowspin"), allowzoom("allowzoom"), minfollow("minfollow"), maxfollow("maxfollow"), cellsize("cellsize");

	orthographic_=eventData[orthographic].GetBool();
	if(orthographic_)
	{
		camera_->SetOrthographic(true);
		Graphics *graphics=context_->GetSubsystem<Graphics>();
		cellsize_=eventData[cellsize].GetUInt();
		//camera_->SetOrthoSize(Vector2(graphics->GetWidth()/(cellsize_*std::sqrt(2.0f)), graphics->GetHeight()/(cellsize_*std::sqrt(2.0f))));
		camera_->SetOrthoSize(graphics->GetWidth()/(cellsize_*std::sqrt(2.0f)));
	}

	rotangle_=eventData[rotangle].GetFloat();
	camangle_=eventData[pitchangle].GetFloat();
	allowspin_=eventData[allowspin].GetBool();
	allowzoom_=eventData[allowzoom].GetBool();
	if(allowzoom_)
	{
		minfollow_=eventData[minfollow].GetFloat();
		maxfollow_=eventData[maxfollow].GetFloat();
	}
}

void ThirdPersonCamera::SetOrthographic(bool ortho)
{
	if(ortho)
	{
		Graphics *graphics=context_->GetSubsystem<Graphics>();
		camera_->SetOrthographic(true);
		//camera_->SetOrthoSize(Vector2(graphics->GetWidth()/cellsize_, graphics->GetHeight()/cellsize_));
		camera_->SetOrthoSize(graphics->GetWidth()/(cellsize_*std::sqrt(2.0f)));
	}
	else camera_->SetOrthographic(false);
}

void ThirdPersonCamera::Start()
{
	SubscribeToEvent(StringHash("ShakeCamera"), URHO3D_HANDLER(ThirdPersonCamera,HandleShakeCamera));
	SubscribeToEvent(StringHash("RequestMouseRay"), URHO3D_HANDLER(ThirdPersonCamera,HandleRequestMouseRay));
	SubscribeToEvent(StringHash("CameraSetPosition"), URHO3D_HANDLER(ThirdPersonCamera,HandleSetCameraPosition));
	SubscribeToEvent(StringHash("CameraResetPosition"), URHO3D_HANDLER(ThirdPersonCamera,HandleResetCameraPosition));
	SubscribeToEvent(StringHash("CameraGetMouseGround"), URHO3D_HANDLER(ThirdPersonCamera,HandleRequestMouseGround));
	SubscribeToEvent(StringHash("RequestCameraRotation"), URHO3D_HANDLER(ThirdPersonCamera,HandleRequestCameraRotation));

	curfollow_=follow_;
	shakenode_=node_->CreateChild("ShakeNode", LOCAL);
	anglenode_=shakenode_->CreateChild("AngleNode", LOCAL);
	cameranode_=anglenode_->CreateChild("CameraNode", LOCAL);
	camera_=cameranode_->CreateComponent<Camera>();

	if(orthographic_)
	{
		Graphics *graphics=context_->GetSubsystem<Graphics>();
		camera_->SetOrthographic(true);
		//camera_->SetOrthoSize(Vector2(graphics->GetWidth()/cellsize_, graphics->GetHeight()/cellsize_));
		camera_->SetOrthoSize(graphics->GetWidth()/(cellsize_*std::sqrt(2.0f)));
	}

	viewport_=new Viewport(context_, node_->GetScene(), camera_);
	Renderer *renderer=context_->GetSubsystem<Renderer>();
	renderer->SetViewport(0,viewport_);

	node_->SetRotation(Quaternion(rotangle_, Vector3(0,1,0)));
	cameranode_->SetPosition(Vector3(0,0,-follow_));
	anglenode_->SetRotation(Quaternion(camangle_, Vector3(1,0,0)));
	node_->SetPosition(Vector3(0,0,0));
	camera_->SetFarClip(clipdist_);
}

Ray ThirdPersonCamera::GetMouseRay()
{
	IntVector2 mousepos;
	Input *input=context_->GetSubsystem<Input>();
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=context_->GetSubsystem<UI>()->GetCursorPosition();
	Graphics *graphics=context_->GetSubsystem<Graphics>();
	return camera_->GetScreenRay((float)mousepos.x_/(float)graphics->GetWidth(), (float)mousepos.y_/(float)graphics->GetHeight());
}

Ray ThirdPersonCamera::GetScreenRay(int mx, int my)
{
	Graphics *graphics=context_->GetSubsystem<Graphics>();
	return camera_->GetScreenRay((float)mx/(float)graphics->GetWidth(), (float)my/(float)graphics->GetHeight());
}

Vector2 ThirdPersonCamera::GetMouseGround()
{
	Ray ray=GetMouseRay();
	float hitdist=ray.HitDistance(Plane(Vector3(0,1,0), Vector3(0,0,0)));
	float dx=(ray.origin_.x_+ray.direction_.x_*hitdist);
	float dz=(ray.origin_.z_+ray.direction_.z_*hitdist);
	return Vector2(dx,dz);
}

bool ThirdPersonCamera::PickGround(Vector3 &ground, int mx, int my, float maxdistance)
{
	Scene *scene=node_->GetScene();
	Vector3 hitPos(0,0,0);
	Drawable *hitDrawable=0;
	UI *ui=context_->GetSubsystem<UI>();
	Input *input=context_->GetSubsystem<Input>();
	Octree *octree=scene->GetComponent<Octree>();


	if(ui->GetCursor() && ui->GetCursor()->IsVisible()==false && input->IsMouseVisible()==false) return false;
	Ray ray=GetScreenRay(mx, my);
	static PODVector<RayQueryResult> result;
	result.Clear();
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, maxdistance, DRAWABLE_GEOMETRY);
	octree->Raycast(query);
	if(result.Size()==0) return false;

	for(unsigned int i=0; i<result.Size(); ++i)
	{
		if(result[i].distance_>=0)
		{
			//hitPos=ray.origin_+ray.direction_*result[i].distance_;
			//ground=Vector2(hitPos.x_, hitPos.z_);
			Node *n=TopLevelNode(result[i].drawable_, scene);
			bool isworld=n->GetVar("world").GetBool();
			if(isworld)
			{
				//Vector3 pos=n->GetPosition();
				ground=ray.origin_+ray.direction_*result[i].distance_;
				return true;
			}
		}
	}

	return false;
}

Vector2 ThirdPersonCamera::GetScreenGround(int mx, int my)
{
	Ray ray=GetScreenRay(mx,my);
	float hitdist=ray.HitDistance(Plane(Vector3(0,1,0), Vector3(0,0,0)));
	if(hitdist==M_INFINITY) return Vector2(0,0);
	Vector3 d=ray.origin_+ray.direction_*hitdist;
	return Vector2(d.x_,d.z_);
}

float ThirdPersonCamera::CameraPick(Ray &ray, float followdist)
{
	Scene *scene=node_->GetScene();
	Octree *octree=scene->GetComponent<Octree>();

	static PODVector<RayQueryResult> result;
	result.Clear();
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, followdist, DRAWABLE_GEOMETRY);
	octree->Raycast(query);
	Log::Write(LOG_INFO, String("Size:")+String(result.Size()));
	if(result.Size()==0) return followdist;

	for(unsigned int i=0; i<result.Size(); ++i)
	{
		Node *n=TopLevelNode(result[i].drawable_, scene);
		Log::Write(LOG_INFO, String("Picked: ")+String(result[i].distance_)+String(" ")+String(followdist));
		/*if(n->GetVars().Find(StringHash("solid"))->second_.GetBool() && result[i].distance_>=0) */return std::min(result[i].distance_-0.0f, followdist);
	}

	return followdist;
}

void ThirdPersonCamera::SpringFollow(float dt)
{
	float df=follow_-curfollow_;
	float af=9.0f*df-6.0f*followvel_;
	followvel_+=dt*af;
	curfollow_+=dt*followvel_;
}

void ThirdPersonCamera::SpringPosition(float dt)
{
	Vector3 d=newpos_-pos_;
	Vector3 a=d*8.0f-posvelocity_*6.0f;
	posvelocity_+=a*dt;
	pos_+=posvelocity_*dt;
}

Node *ThirdPersonCamera::TopLevelNode(Drawable *d, Scene *s)
{
	Node *n=d->GetNode();
	if(!n) return 0;
	while(n->GetParent() != s)
	{
		if(n->GetParent()==0) return 0;
		n=n->GetParent();
	}
	return n;
}


void CombatCameraController::RegisterObject(Context *context)
{
	context->RegisterFactory<CombatCameraController>();
	URHO3D_ACCESSOR_ATTRIBUTE("Offset", GetOffset, SetOffset, float, 0.0f, AM_DEFAULT);
}

CombatCameraController::CombatCameraController(Context *context) : LogicComponent(context), offset_(0)
{
	SetUpdateEventMask(USE_UPDATE);
}

void CombatCameraController::Update(float dt)
{
	static StringHash position("position"), camerasetposition("CameraSetPosition");
	VariantMap vm;
	Vector3 pos=node_->GetPosition();
	vm[position]=Vector3(pos.x_, pos.y_+offset_, pos.z_);
	node_->SendEvent(camerasetposition, vm);
}
