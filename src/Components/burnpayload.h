#pragma once
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
using namespace Urho3D;

class BurnPayload : public LogicComponent
{
	URHO3D_OBJECT(BurnPayload, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	BurnPayload(Context *context);

	void SetBurnAmount(float dmg){damage_=dmg;}
	void SetInterval(float interval){interval_=interval;}
	void SetOwner(Node *n){owner_=n;}
	void SetRadius(float r){radius_=r;}
	void SetDuration(float dur){duration_=dur;}

	protected:
	float interval_;
	float counter_;
	float damage_;
	float radius_;
	float duration_;
	WeakPtr<Node> owner_;

	void HandleTriggerPayload(StringHash eventType, VariantMap &eventData);
	virtual void DelayedStart() override;
};
