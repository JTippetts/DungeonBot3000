#include <Urho3D/Urho3D.h>
#include "RegisterComponents.h"

#include "Components/thirdpersoncamera.h"
#include "Components/rotationsmoothing.h"
#include "Components/combatcontroller.h"
#include "Components/playercontroller.h"
#include "Components/stupidchasecontroller.h"
#include "Components/vitals.h"
#include "Components/lifebubbleui.h"
#include "Components/enemylifebar.h"
#include "Components/burnindicator.h"
#include "Components/enemyondeath.h"
#include "Components/timeddeath.h"
#include "Components/enemyai.h"

void RegisterCustomComponents(Context *context)
{
	ThirdPersonCamera::RegisterObject(context);
	context->RegisterFactory<CombatCameraController>("Custom");
	RotationSmoothing::RegisterObject(context);
	CombatController::RegisterObject(context);
	//PlayerController::RegisterObject(context);
	EnemyAI::RegisterObject(context);
	StupidChaseController::RegisterObject(context);
	PlayerVitals::RegisterObject(context);
	EnemyVitals::RegisterObject(context);
	LifeBubbleUI::RegisterObject(context);
	EnemyLifeBar::RegisterObject(context);
	BurnIndicator::RegisterObject(context);
	EnemyOnDeath::RegisterObject(context);
	TimedDeath::RegisterObject(context);

	//context->RegisterFactory<CombatActionState>();
	context->RegisterFactory<CASPlayerIdle>();
	context->RegisterFactory<CASPlayerMove>();
	context->RegisterFactory<CASPlayerSpinAttack>();
	context->RegisterFactory<CASEnemyIdle>();
	context->RegisterFactory<CASEnemyChase>();
	context->RegisterFactory<CASEnemyKick>();
}
