#pragma once

// Projectile

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
using namespace Urho3D;

class Projectile : public LogicComponent
{
	URHO3D_OBJECT(Projectile, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	Projectile(Context *context);

	void Setup(const Vector3 &start, const Vector3 &end, const float speed, const float archeight);

	protected:
	Vector3 start_, end_;
	float duration_, ttl_;
	float archeight_;

	virtual void Update(float dt) override;
};
