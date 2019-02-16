#pragma once

#include "combatactionstates.h"
#include <Urho3D/Audio/SoundSource.h>

// Player action states

class CASPlayerBase : public CombatActionState
{
	URHO3D_OBJECT(CASPlayerBase, CombatActionState);
	public:
	CASPlayerBase(Context *context);

	CombatActionState *CheckInputs(CombatController *actor);

};

class CASPlayerDead : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerDead, CASPlayerBase);
	public:
	CASPlayerDead(Context *context);
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
};

class CASPlayerIdle : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerIdle, CASPlayerBase);
	public:
	CASPlayerIdle(Context *context);
	virtual void End(CombatController *actor) override;
	virtual void Start(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerMove : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerMove, CASPlayerBase);
	public:
	CASPlayerMove(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;
};

class CASPlayerLoot : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerLoot, CASPlayerBase);
	public:
	CASPlayerLoot(Context *context);

	void SetItem(Node *i)
	{
		item_=i;
	}

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;

	protected:
	WeakPtr<Node> item_;
};

class CASPlayerStairs : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerStairs, CASPlayerBase);
	public:
	CASPlayerStairs(Context *context);

	void SetStairsNode(Node *n){stairs_=n;};

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual bool HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt) override;

	protected:
	WeakPtr<Node> stairs_;
};

class CASPlayerSpinAttack : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerSpinAttack, CASPlayerBase);
	public:
	CASPlayerSpinAttack(Context *context);

	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;
	virtual void HandleTrigger(CombatController *actor, String animname, unsigned int value) override;

	protected:
	SoundSource *swing_;
	SharedPtr<Sound> snd_;
};

class CASPlayerLaserBeam : public CASPlayerBase
{
	URHO3D_OBJECT(CASPlayerLaserBeam, CASPlayerBase);
	public:
	CASPlayerLaserBeam(Context *context);
	virtual void Start(CombatController *actor) override;
	virtual void End(CombatController *actor) override;
	virtual CombatActionState *Update(CombatController *actor, float dt) override;

	protected:
	Node *endburst_, *beam_;
	SoundSource *swing_;

	Vector3 lastendpos_, endpos_;
	float interval_, timetopulse_, phase_;

	Vector3 GetEndPoint(Node *node);
};
