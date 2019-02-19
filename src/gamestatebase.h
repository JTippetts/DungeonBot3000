#pragma once

// Base class for a game state

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

class GameStateBase : public Object
{
	URHO3D_OBJECT(GameStateBase, Object);
	public:
	GameStateBase(Context *context) : Object(context){}

	VariantMap &GetData(){return data_;}
	void SetData(const VariantMap &data){data_=data;}
	Scene *GetScene(){return scene_;}

	virtual void Start()=0;
	virtual void Stop(){if(scene_) scene_->Remove(); scene_.Reset();}

	const BoundingBox &GetBoundingBox() const {return bbox_;}

	protected:
	VariantMap data_;
	SharedPtr<Scene> scene_;
	BoundingBox bbox_;
};
