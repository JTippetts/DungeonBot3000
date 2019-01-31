#pragma once

#include "combatactionstates.h"

// Player action states
class CASPlayerIdle : public CombatActionState
{
	URHO3D_OBJECT(CASPlayerIdle, CombatActionState);
	public:
	CASPlayerIdle(Context *context);
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerMove : public CombatActionState
{
	URHO3D_OBJECT(CASPlayerMove, CombatActionState);
	public:
	CASPlayerMove(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerSpinAttack : public CombatActionState
{
	URHO3D_OBJECT(CASPlayerSpinAttack, CombatActionState);
	public:
	CASPlayerSpinAttack(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;
};

class CASPlayerLaserBeam : public CombatActionState
{
	URHO3D_OBJECT(CASPlayerLaserBeam, CombatActionState);
	public:
	CASPlayerLaserBeam(Context *context);
	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;

	protected:
	Node *endburst_, *beam_;

	Vector3 lastendpos_, endpos_;
	float interval_, timetopulse_, phase_;

	Vector3 GetEndPoint(Node *node);
};
