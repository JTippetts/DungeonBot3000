// Third person camera
#ifndef THIRDPERSONCAMERA_H
#define THIRDPERSONCAMERA_H
#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

class ThirdPersonCamera : public LogicComponent
{
	URHO3D_OBJECT(ThirdPersonCamera, LogicComponent);

	public:
	ThirdPersonCamera(Context *context);
	static void RegisterObject(Context *context);
	virtual void Update(float dt);

	void SetCellSize(unsigned int s){cellsize_=s;SetOrthographic(true);}
	unsigned int GetCellSize() const{return cellsize_;}
	void SetCamAngle(float a){camangle_=a;}
	float GetCamAngle() const{return camangle_;}
	void SetRotAngle(float a){rotangle_=a;}
	float GetRotAngle() const{return rotangle_;}
	void SetClipDist(float c){clipdist_=c;camera_->SetFarClip(clipdist_);}
	float GetClipDist() const{return clipdist_;}
	void SetClipCamera(bool c){clipcamera_=c;}
	bool GetClipCamera() const{return clipcamera_;}
	void SetSpringTrack(bool s){springtrack_=s;}
	bool GetSpringTrack() const{return springtrack_;}
	void SetAllowPitch(bool a){allowpitch_=a;}
	bool GetAllowPitch() const{return allowpitch_;}
	void SetAllowSpin(bool a){allowspin_=a;}
	bool GetAllowSpin() const{return allowspin_;}
	void SetAllowZoom(bool a){allowzoom_=a;}
	bool GetAllowZoom() const{return allowzoom_;}
	void SetOrthographic(bool o);
	bool GetOrthographic() const{return orthographic_;}
	void SetOffset(float o){offset_=o;}
	float GetOffset() const{return offset_;}
	void SetMinFollow(float f){minfollow_=f; if(follow_<minfollow_) follow_=minfollow_;}
	float GetMinFollow() const{return minfollow_;}
	void SetMaxFollow(float f){maxfollow_=f;if(follow_>maxfollow_) follow_=maxfollow_;}
	float GetMaxFollow() const{return maxfollow_;}

	void SetFollow(float f);
	Viewport *GetViewport(){return viewport_.Get();}

	Ray GetMouseRay();
	Ray GetScreenRay(int mx, int my);
	Vector2 GetMouseGround();
	bool PickGround(Vector2 &ground, int mx, int my, float maxdistance=1000.0f);
	Vector2 GetScreenGround(int mx, int my);
	float CameraPick(Ray &ray, float followdist);


	protected:
	void HandleSetCameraPosition(StringHash eventType, VariantMap &eventData);
	void HandleResetCameraPosition(StringHash eventType, VariantMap &eventData);
	void HandleRequestCameraRotation(StringHash eventType, VariantMap &eventData);
	void HandleShakeCamera(StringHash eventType, VariantMap &eventData);
	void HandleRequestMouseGround(StringHash eventType, VariantMap &eventData);
	void HandleRequestMouseRay(StringHash eventType, VariantMap &eventData);
	void HandleSetCamera(StringHash eventType, VariantMap &eventData);
	virtual void Start();


	void SpringFollow(float dt);
	void SpringPosition(float dt);

	Node *TopLevelNode(Drawable *d, Scene *s);

	unsigned int cellsize_;
	float camangle_, rotangle_;
	float follow_, minfollow_, maxfollow_;
	float clipdist_;
	bool clipcamera_;
	bool springtrack_;
	bool allowspin_, allowpitch_, allowzoom_;
	bool orthographic_;

	float curfollow_;
	float followvel_;
	Vector3 pos_;
	Vector3 newpos_;
	Vector3 posvelocity_;
	float offset_;

	float shakemagnitude_, shakespeed_, shaketime_, shakedamping_;

	SharedPtr<Node> shakenode_, anglenode_, cameranode_;
	SharedPtr<Camera> camera_;
	SharedPtr<Viewport> viewport_;


};

class CombatCameraController : public LogicComponent
{
	URHO3D_OBJECT(CombatCameraController, LogicComponent);

	public:
	CombatCameraController(Context *context);
	virtual void Update(float dt);

	void SetOffset(float o){offset_=o;}
	float GetOffset(){return offset_;}

	protected:
	virtual void Start();
	virtual void OnMarkedDirty(Node *n);

	void HandleCombatTurnBegin(StringHash eventType, VariantMap& eventData);
	void HandleCombatTurnEnd(StringHash eventType, VariantMap& eventData);

	bool active_;
	float offset_;

};

#endif
