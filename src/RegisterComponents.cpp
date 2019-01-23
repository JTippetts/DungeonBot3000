#include <Urho3D/Urho3D.h>
#include "RegisterComponents.h"

#include "Components/thirdpersoncamera.h"
#include "Components/rotationsmoothing.h"
#include "Components/combatcontroller.h"
#include "Components/playercontroller.h"
#include "Components/stupidchasecontroller.h"
#include "Components/vitals.h"

void RegisterCustomComponents(Context *context)
{
	ThirdPersonCamera::RegisterObject(context);
	context->RegisterFactory<CombatCameraController>("Custom");
	RotationSmoothing::RegisterObject(context);
	CombatController::RegisterObject(context);
	PlayerController::RegisterObject(context);
	StupidChaseController::RegisterObject(context);
	PlayerVitals::RegisterObject(context);
}
