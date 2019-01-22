// Combat Controller
#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Navigation/CrowdAgent.h>

using namespace Urho3D;

class CombatController : public LogicComponent
{
	URHO3D_OBJECT(CombatController, LogicComponent);

	public:
	static void RegisterObject(Context *context);
	CombatController(Context *context);

	void SetObjectPath(String op);
	const String GetObjectPath() const;
	void SetAnimPath(String ap);
	const String GetAnimPath() const;

	void MoveTo(Vector3 target);
	void SetPushiness(NavigationPushiness pushy);

	protected:
	virtual void Update(float dt) override;
	void HandleCrowdAgentReposition(StringHash eventType, VariantMap &eventData);

	String objectpath_, animpath_;
};
