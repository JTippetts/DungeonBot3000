#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/UI/BorderImage.h>

using namespace Urho3D;

// Implement a scene fader system

class SceneFade : public Object
{
	URHO3D_OBJECT(SceneFade, Object);
	public:
	SceneFade(Context *context);

	bool IsFading();

	void FadeIn(float duration);
	void FadeOut(float duration);

	void HandleUpdate(StringHash eventType, VariantMap &eventData);

	protected:
	float duration_, counter_;
	bool fadein_;

	SharedPtr<BorderImage> element_;
};
