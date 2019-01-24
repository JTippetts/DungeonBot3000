// Enemy life bar component
// Implement a life bar that floats above an enemy's head

#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class EnemyLifeBar : public LogicComponent
{
	URHO3D_OBJECT(EnemyLifeBar, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	EnemyLifeBar(Context *context);

	void SetOffset(double f);
	const double GetOffset() const;

	protected:
	SharedPtr<UIElement> element_;
	double offset_;

	virtual void DelayedStart() override;
	virtual void Update(float dt) override;
	void HandleLifeDepleted(StringHash eventType, VariantMap &eventData);
};
