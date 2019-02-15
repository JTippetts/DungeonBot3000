#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class PlayerDeath : public LogicComponent
{
	URHO3D_OBJECT(PlayerDeath, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	PlayerDeath(Context *context);

	protected:
	void HandleLifeDepleted(StringHash eventType, VariantMap &eventData);
	void HandleRespawn(StringHash eventType, VariantMap &eventData);
	void HandleReturn(StringHash eventType, VariantMap &eventData);
	void HandleExit(StringHash eventType, VariantMap &eventData);
	virtual void DelayedStart() override;
	virtual void Stop() override;

	SharedPtr<UIElement> element_;
	bool triggered_;
};
