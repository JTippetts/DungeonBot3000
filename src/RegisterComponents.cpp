#include <Urho3D/Urho3D.h>
#include "RegisterComponents.h"

#include "Components/thirdpersoncamera.h"
#include "Components/rotationsmoothing.h"

void RegisterCustomComponents(Context *context)
{
	ThirdPersonCamera::RegisterObject(context);
	context->RegisterFactory<CombatCameraController>("Custom");
	RotationSmoothing::RegisterObject(context);
}
