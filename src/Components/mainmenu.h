#pragma once

// Main Menu class

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class MainMenu : public LogicComponent
{
	URHO3D_OBJECT(MainMenu, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	MainMenu(Context *context);

	protected:
	SharedPtr<UIElement> element_;

	void HandlePlay(StringHash eventType, VariantMap &eventData);
	void HandleExit(StringHash eventType, VariantMap &eventData);
	virtual void Update(float dt) override;
	virtual void DelayedStart() override;
	virtual void Stop() override;
};
