#pragma once

// Vitals component
// Handle life

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "../stats.h"
#include "../combat.h"

#include <unordered_map>
#include <list>

using namespace Urho3D;

struct BurnDoT
{
	double ttl_;
	double counter_;
	double dps_;
	WeakPtr<Node> owner_;
	StatSetCollectionSnapshot ownerstats_;
};

struct HealHoT
{
	double ttl_;
	double counter_;
	double hps_;
	StatSetCollectionSnapshot ownerstats_;
};

class PlayerVitals : public LogicComponent
{
	URHO3D_OBJECT(PlayerVitals, LogicComponent);
	public:
	static void RegisterObject(Context *context);
	PlayerVitals(Context *context);

	void SetStats(StatSetCollection *vitalsstats);

	double GetCurrentLife();
	double GetMaximumLife();
	double GetEnergy();


	protected:
	StatSetCollection *vitalsstats_;
	double currentlife_, maximumlife_;
	double energy_;

	std::unordered_map<unsigned int, BurnDoT> dots_;
	std::list<HealHoT> hots_;

	virtual void Update(float dt) override;
	void UpdateDoTs(float dt);
	void UpdateHoTs(float dt);

	void ApplyDamageList(const DamageValueList &dmg);
};

