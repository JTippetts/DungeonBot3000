#pragma once

// Stupid chase controller

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Navigation/CrowdAgent.h>

#include "enemyai.h"
#include "../playerdata.h"
#include "combatcontroller.h"

using namespace Urho3D;

float rollf(float,float);

class CombatActionState;
/*

class StupidChaseController : public EnemyAI
{
	URHO3D_OBJECT(StupidChaseController, EnemyAI);

	public:
	static void RegisterObject(Context *context)
	{
		context->RegisterFactory<StupidChaseController>("Logic");
	}

	StupidChaseController(Context *context) : EnemyAI(context)
	{
	}

	virtual CombatActionState *Callback(CombatActionState *state) override
	{
		//Log::Write(LOG_INFO, "0");
		auto actor=node_->GetComponent<CombatController>();
		if(!actor)
		{
			//Log::Write(LOG_INFO, "1");
			return nullptr;
		}
		if(state==actor->GetState<CASEnemyIdle>())
		{
			//Log::Write(LOG_INFO, "2");
			return actor->GetState<CASEnemyChase>();
		}

		if(state==actor->GetState<CASEnemyChase>())
		{
			//Log::Write(LOG_INFO, "3");
			auto pd = GetSubsystem<PlayerData>();
			auto playerpos = pd->GetPlayerNode()->GetWorldPosition();
			Vector3 delta = playerpos - node_->GetWorldPosition();
			if(delta.Length() < 5)
			{
				return actor->GetState<CASEnemyKick>();
			}
		}
		return nullptr;
	}
};
*/
