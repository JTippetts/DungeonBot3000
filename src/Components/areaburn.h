#pragma once
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
using namespace Urho3D;

class AreaBurn : public LogicComponent
{
	URHO3D_OBJECT(AreaBurn, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	AreaBurn(Context *context);

	void SetBurnAmount(float dmg);
	void SetInterval(float interval);
	void SetOwner(Node *n){owner_=n;}
	void SetRadius(float r){radius_=r;}

	protected:
	float interval_;
	float counter_;
	float damage_;
	float radius_;
	WeakPtr<Node> owner_;

	virtual void Update(float dt) override;
};
