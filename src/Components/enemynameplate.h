#pragma once

// Enemy nameplate
// Enemy name and life bar, plus mods list

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class EnemyNameplate : public LogicComponent
{
	URHO3D_OBJECT(EnemyNameplate, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	EnemyNameplate(Context *context);

	void SetName(const String &name);
	const String &GetName() const;
	void SetNameColor(const Color &color);
	const Color &GetNameColor() const;

	void SetHealthbar(float ratio);

	void SetVisible(bool vis);

	void AddMod(const String &name);

	protected:
	SharedPtr<UIElement> element_;
	String name_;
	Color color_;

	virtual void Start() override;
	virtual void Update(float dt) override;
	virtual void DelayedStart() override;
	virtual void Stop() override;
	void HandleHover(StringHash eventType, VariantMap &eventData);
	void HandleUnHover(StringHash eventType, VariantMap &eventData);
};
