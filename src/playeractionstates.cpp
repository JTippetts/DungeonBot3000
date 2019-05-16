#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "playerdata.h"
#include "Components/vitals.h"

#include "playeractionstates.h"
#include "Components/itemnametagcontainer.h"
#include "Components/dropitem.h"
#include "Components/levelchanger.h"
#include "playerinventory.h"

Node *TopLevelNode(Drawable *d, Scene *s)
{
	Node *n=d->GetNode();
	if(!n) return 0;
	while(n->GetParent() != s)
	{
		if(n->GetParent()==0) return 0;
		n=n->GetParent();
	}
	return n;
}

//// Base for player states
CASPlayerBase::CASPlayerBase(Context *context) : CombatActionState(context)
{
}

CombatActionState *CASPlayerBase::CheckInputs(CombatController *actor)
{
	auto input=GetSubsystem<Input>();
	auto node=actor->GetNode();
	auto pd=GetSubsystem<PlayerData>();
	auto ui=GetSubsystem<UI>();
	auto &is=pd->GetInventoryScreen();

	IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=ui->GetCursorPosition();

	auto hudlayer=ui->GetRoot()->GetChild("HUDLayer", true);
	if(hudlayer)
	{
		unsigned int numchildren=hudlayer->GetNumChildren();
		for(unsigned int c=0; c<numchildren; ++c)
		{
			auto child=hudlayer->GetChild(c);
			auto rect=child->GetCombinedScreenRect();
			if(child->IsVisible() && rect.IsInside(mousepos)==INSIDE) return nullptr;
		}
	}
	else
	{
		Log::Write(LOG_INFO, "Couldn't obtain hudlayer");
	}

	/*auto vitals=node->GetComponent<PlayerVitals>();
	if(vitals)
	{
		if(vitals->GetCurrentLife()<=0)
		{
			return actor->GetState<CASPlayerDead>();
		}
	}*/

	if(input->GetMouseButtonPress(MOUSEB_LEFT) && !is.HasItemInHand())
	{
		//auto nametags=GetSubsystem<ItemNameTagContainer>();
		auto nametags=node->GetScene()->GetComponent<ItemNameTagContainer>();
		if(nametags)
		{
			auto nametag=nametags->GetHoveredTag();
			if(nametag)
			{
				auto loot=actor->GetState<CASPlayerLoot>();
				loot->SetItem(nametag->GetNode());
				return loot;
			}
		}
	}

	if(input->GetMouseButtonPress(MOUSEB_RIGHT))
	{
		PlayerAttack a=pd->GetAttack();
		switch(a)
		{
			case PASpinAttack:
			{
				auto stats=pd->GetStatSetCollection(EqBlade, "SpinAttack");
				double energycost=GetStatValue(stats, "EnergyCost");
				double energy=pd->GetEnergy();
				if(energy>=energycost) return actor->GetState<CASPlayerSpinAttack>();
			} break;
			case PALaserBeam:
			{
				auto stats=pd->GetStatSetCollection(EqTurret, "LaserBeam");
				double energycost=GetStatValue(stats, "EnergyCost");
				double energy=pd->GetEnergy();
				if(energy>=energycost) return actor->GetState<CASPlayerLaserBeam>();
			} break;
			case PACharge:
			{
				// First, check cooldown
				static StringHash Charge("Charge");
				float cd=actor->GetCooldown(Charge);
				if(cd<=0.0)
				{
					auto stats=pd->GetStatSetCollection(EqDriveSystem, "Charge");
					double energycost=GetStatValue(stats, "EnergyCost");
					double energy=pd->GetEnergy();
					if(energy>=energycost)
					{
						auto charge = actor->GetState<CASPlayerCharge>();
						return charge;
					}
				}
			} break;
		};
	}
	else if(input->GetMouseButtonPress(MOUSEB_LEFT) && !is.HasItemInHand())
	{
		// Look for stairs
		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		auto ray=cam->GetMouseRay();
		auto octree=node->GetScene()->GetComponent<Octree>();
		auto scene=node->GetScene();

		PODVector<RayQueryResult> result;
		result.Clear();
		RayOctreeQuery query(result, ray, RAY_TRIANGLE, 300.0f, DRAWABLE_GEOMETRY);
		octree->Raycast(query);
		if(result.Size()>0)
		{
			for(unsigned int c=0; c<result.Size(); ++c)
			{
				Node *n = result[c].drawable_->GetNode();
				if(n && n->GetComponent<LevelChanger>()!=nullptr)
				{
					CASPlayerStairs *stairs = actor->GetState<CASPlayerStairs>();
					stairs->SetStairsNode(n);
					return stairs;
				}
			}
		}
		// Do walk
		return actor->GetState<CASPlayerMove>();
	}
	return nullptr;
}


//////// Dead
CASPlayerDead::CASPlayerDead(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerDead::End(CombatController *actor)
{
}

void CASPlayerDead::Start(CombatController *actor)
{
}

CombatActionState *CASPlayerDead::Update(CombatController *actor, float dt)
{
	return nullptr;
}

/////// Idle
CASPlayerIdle::CASPlayerIdle(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerIdle::Start(CombatController *actor)
{

	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
	}
}

void CASPlayerIdle::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
		}
	}
}

CombatActionState *CASPlayerIdle::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=node->GetSubsystem<Input>();
	auto pd=node->GetSubsystem<PlayerData>();

	// Check to see if we clicked on something
	auto vitals=node->GetComponent<PlayerVitals>();
	if(vitals)
	{
		if(vitals->GetCurrentLife()<=0)
		{
			return actor->GetState<CASPlayerDead>();
		}
	}
	return CheckInputs(actor);

}

bool CASPlayerIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}

	return true;
}

////// PlayerMove
CASPlayerMove::CASPlayerMove(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerMove::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
		}
	}
}

void CASPlayerMove::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani");
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASPlayerMove::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();
	auto pd=actor->GetSubsystem<PlayerData>();


	auto ci=CheckInputs(actor);
	if(ci && ci!=this)
	{
		return ci;
	}
	else if(input->GetMouseButtonDown(MOUSEB_LEFT))
	{
		auto vitals=node->GetComponent<PlayerVitals>();
		if(vitals)
		{
			if(vitals->GetCurrentLife()<=0)
			{
				return actor->GetState<CASPlayerDead>();
			}
		}
		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
		ca->SetTargetPosition(gp);

		return nullptr;
	}
	else
	{
		auto vitals=node->GetComponent<PlayerVitals>();
		if(vitals)
		{
			if(vitals->GetCurrentLife()<=0)
			{
				return actor->GetState<CASPlayerDead>();
			}
		}
		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			Vector3 vel=ca->GetActualVelocity();
			if(vel.Length() < ca->GetRadius())
			{
				return actor->GetState<CASPlayerIdle>();
			}
		}
	}

	return nullptr;
}

bool CASPlayerMove::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	return false;
}


/////////// Loot
CASPlayerLoot::CASPlayerLoot(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerLoot::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
		}
	}
}

void CASPlayerLoot::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani");
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASPlayerLoot::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();
	if(!item_) return actor->GetState<CASPlayerIdle>();
	auto pd=actor->GetSubsystem<PlayerData>();


	auto ci=CheckInputs(actor);
	if(ci)
	{
		return ci;
	}
	else if(input->GetMouseButtonDown(MOUSEB_LEFT))
	{
		Vector3 ground=item_->GetWorldPosition();

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		Vector3 delta=ground-node->GetWorldPosition();
		if(delta.Length() < ca->GetRadius())
		{
			// At item, loot it
			auto itemdrop = item_->GetComponent<DropItemContainer>();
			if(itemdrop)
			{
				auto pd=GetSubsystem<PlayerData>();
				auto &is=pd->GetInventoryScreen();
				if(is.IsVisible())
				{
					// Inventory screen visible, so check to see if there is an item in hand.
					// If item in hand, just do nothing and bail.
					if(!is.HasItemInHand())
					{
						is.PutItemInHand(itemdrop->GetItem());
						itemdrop->ClearItem();
						item_->GetComponent<ItemNameTag>()->RemoveTag();
						item_->Remove();
					}
					else
					{
						// Item already in hand, do nothing
					}
				}
				else
				{
					auto pi=GetSubsystem<PlayerInventory>();
					if(pi->AddItem(itemdrop->GetItem(), true))
					{
						itemdrop->ClearItem();
						item_->GetComponent<ItemNameTag>()->RemoveTag();
						item_->Remove();
					}
					else
					{
						Log::Write(LOG_INFO, "Not enough inventory space");
					}
				}
			}
			return actor->GetState<CASPlayerIdle>();
		}

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		//Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
		ca->SetTargetPosition(ground);

		return nullptr;
	}
	else
	{
		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			Vector3 vel=ca->GetActualVelocity();
			if(vel.Length() < ca->GetRadius())
			{
				auto itemdrop = item_->GetComponent<DropItemContainer>();
				if(itemdrop)
				{
					auto pd=GetSubsystem<PlayerData>();
					auto &is=pd->GetInventoryScreen();
					if(is.IsVisible())
					{
						// Inventory screen visible, so check to see if there is an item in hand.
						// If item in hand, just do nothing and bail.
						if(!is.HasItemInHand())
						{
							is.PutItemInHand(itemdrop->GetItem());
							itemdrop->ClearItem();
							item_->GetComponent<ItemNameTag>()->RemoveTag();
							item_->Remove();
						}
						else
						{
							// Item already in hand, do nothing
						}
					}
					else
					{
						auto pi=GetSubsystem<PlayerInventory>();
						if(pi->AddItem(itemdrop->GetItem(), true))
						{
							itemdrop->ClearItem();
							item_->GetComponent<ItemNameTag>()->RemoveTag();
							item_->Remove();
						}
						else
						{
							Log::Write(LOG_INFO, "Not enough inventory space");
						}
					}
				}
				return actor->GetState<CASPlayerIdle>();
			}
		}
	}

	return nullptr;
}

bool CASPlayerLoot::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	return false;
}


//////////// Use Stairs
CASPlayerStairs::CASPlayerStairs(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerStairs::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
		}
	}
}

void CASPlayerStairs::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani");
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASPlayerStairs::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();
	if(!stairs_) return actor->GetState<CASPlayerIdle>();
	auto pd=actor->GetSubsystem<PlayerData>();
	auto changer=stairs_->GetComponent<LevelChanger>();
	if(!changer) return actor->GetState<CASPlayerIdle>();
	auto ca=node->GetComponent<CrowdAgent>();

	auto vitals=node->GetComponent<PlayerVitals>();
	if(vitals)
	{
		if(vitals->GetCurrentLife()<=0)
		{
			return actor->GetState<CASPlayerDead>();
		}
	}

	auto delta=stairs_->GetWorldPosition()-node->GetWorldPosition();
	float len=delta.Length();
	if(len <= changer->GetRadius() + ca->GetRadius())
	{
		// Change levels
		changer->Use();
		return actor->GetState<CASPlayerIdle>();
	}

	auto ci=CheckInputs(actor);
	if(ci)
	{
		return ci;
	}
	else
	{
		Vector3 ground=stairs_->GetWorldPosition();

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		ground=nav->FindNearestPoint(ground, Vector3(40,2,40));
		ca->SetTargetPosition(ground);

		return nullptr;
	}

	return nullptr;
}

bool CASPlayerStairs::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	return false;
}

////////////// SpinAttack
CASPlayerSpinAttack::CASPlayerSpinAttack(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerSpinAttack::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Walk.ani", 0, true, 0.1f);
			ac->Play(actor->GetAnimPath() + "/Models/Spin.ani", 0, true, 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
		}
		auto cache=GetSubsystem<ResourceCache>();
		snd_= cache->GetResource<Sound>("Sound/swing2.ogg");
		swing_ = node->GetScene()->CreateComponent<SoundSource>();

		if (snd_)
		{

			snd_->SetLooped(true);
			swing_->Play(snd_);
			swing_->SetGain(0.75f);
		}
	}
}

void CASPlayerSpinAttack::End(CombatController *actor)
{
	auto node=actor->GetNode();
	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Walk.ani", 0.1f);
			ac->Stop(actor->GetAnimPath() + "/Models/Spin.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
		if(swing_)
		{
			swing_->Remove();
			swing_=nullptr;
		}
		if(snd_) snd_.Reset();
	}
}

CombatActionState *CASPlayerSpinAttack::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();
	auto pd=actor->GetSubsystem<PlayerData>();

	auto vitals=node->GetComponent<PlayerVitals>();
	if(vitals)
	{
		if(vitals->GetCurrentLife()<=0)
		{
			return actor->GetState<CASPlayerDead>();
		}
	}

	if(input->GetMouseButtonDown(MOUSEB_RIGHT))
	//if(input->GetKeyDown(KEY_Q))
	{
		double energy=pd->GetEnergy();
		auto stats=pd->GetStatSetCollection(EqBlade, "SpinAttack");
		double cost=GetStatValue(stats, "EnergyCost");

		if(pd->GetAttack() != PASpinAttack || cost*dt>energy)
		{
			if(input->GetMouseButtonDown(MOUSEB_LEFT))
			{
				return actor->GetState<CASPlayerMove>();
			}
			else return actor->GetState<CASPlayerIdle>();
		}
		pd->SetEnergy(energy-cost*dt);

		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();
		auto pd=node->GetSubsystem<PlayerData>();
		auto ca=node->GetComponent<CrowdAgent>();

		StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
		double movespeed=GetStatValue(ssc, "MovementSpeed");
		ca->SetMaxSpeed(movespeed);
		Vector3 gp=nav->FindNearestPoint(Vector3(ground.x_,0,ground.y_), Vector3(10,10,10));
		ca->SetTargetPosition(gp);

		double attackspeed=GetStatValue(ssc, "AttackSpeed");
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->SetSpeed(actor->GetAnimPath() + "/Models/Spin.ani", attackspeed);
		}

		return nullptr;
	}
	else
	{
		// Not walking anymore, return moving or idle state
		if(input->GetMouseButtonDown(MOUSEB_LEFT))
		{
			return actor->GetState<CASPlayerMove>();
		}

		return actor->GetState<CASPlayerIdle>();
	}
}

void CASPlayerSpinAttack::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	if(animname=="Spin")
	{
		auto node=actor->GetNode();
		PODVector<Node *> dudes;
		node->GetScene()->GetChildrenWithComponent<EnemyVitals>(dudes, false);
		Vector3 mypos=node->GetWorldPosition();
		auto cache=node->GetSubsystem<ResourceCache>();

		for(auto i=dudes.Begin(); i!=dudes.End(); ++i)
		{
			Vector3 pos=(*i)->GetWorldPosition();
			Vector3 delta=mypos-pos;
			if(delta.Length() < 8)
			{
				auto myvitals = node->GetComponent<PlayerVitals>();
				auto vtls = (*i)->GetComponent<EnemyVitals>();
				if(vtls && myvitals)
				{
					auto pd=node->GetSubsystem<PlayerData>();
					StatSetCollection ssc=pd->GetStatSetCollection(EqBlade, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);
					vtls->ApplyDamageList(myvitals,ssc,dmg);
				}

				auto* sound = cache->GetResource<Sound>("Sound/sword-unsheathe5.ogg");

				if (sound)
				{
					auto* soundSource = node->GetScene()->CreateComponent<SoundSource>();
					soundSource->SetAutoRemoveMode(REMOVE_COMPONENT);
					soundSource->Play(sound);
					soundSource->SetGain(0.5f);
				}
			}
		}
	}
}

//////// Friggin laser beams
CASPlayerLaserBeam::CASPlayerLaserBeam(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerLaserBeam::Start(CombatController *actor)
{
	auto cache = GetSubsystem<ResourceCache>();
	auto node = actor->GetNode();
	auto ac = node->GetComponent<AnimationController>();
	auto scene = node->GetScene();
	if(ac)
	{
		ac->StopAll(0.1f);
		ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
	}

	if(!endburst_)
	{
		endburst_ = scene->CreateChild();
		auto emitter = endburst_->CreateComponent<ParticleEmitter>();
		auto pe = cache->GetResource<ParticleEffect>("Effects/laserbeamparticle.xml");
		if(pe)
		{
			emitter->SetEffect(pe);
		}

		auto lnode = endburst_->CreateChild("LightNode");
		lnode->SetPosition(Vector3(0,8,0));
		auto ll = lnode->CreateComponent<Light>();
		ll->SetLightType(LIGHT_POINT);
		ll->SetRange(16);
		ll->SetEnabled(true);
		ll->SetColor(Color(1.5,1.2,1));
	}

	endburst_->GetComponent<ParticleEmitter>()->SetEmitting(true);
	endburst_->GetChild("LightNode", true)->SetEnabled(true);

	if(!beam_)
	{
		beam_ = scene->CreateChild();


		auto beambb = beam_->CreateComponent<StaticModel>();
		if(beambb)
		{
			beambb->SetModel(cache->GetResource<Model>("Effects/Beam.mdl"));
			beambb->SetMaterial(cache->GetResource<Material>("Effects/flame2.xml"));
		}
	}

	beam_->SetEnabled(true);

	lastendpos_ = endpos_ = GetEndPoint(node);
	timetopulse_=0;
	phase_=0;

	auto *swing = cache->GetResource<Sound>("Sound/Spell_03.ogg");
	if (swing)
	{
		swing->SetLooped(true);
		swing_ = node->GetScene()->CreateComponent<SoundSource>();
		swing_->Play(swing);
		swing_->SetGain(0.75f);
	}
}

void CASPlayerLaserBeam::End(CombatController *actor)
{
	auto node = actor->GetNode();
	auto ac = node->GetComponent<AnimationController>();
	if(ac)
	{
		ac->Stop(actor->GetAnimPath() + "/Models/Idle.ani", 0.1f);
	}

	//endburst_->Remove();
	endburst_->GetComponent<ParticleEmitter>()->SetEmitting(false);
	endburst_->GetChild("LightNode", true)->SetEnabled(false);

	//beam_->Remove();
	beam_->SetEnabled(false);

	if(swing_)
	{
		swing_->Remove();
		swing_=nullptr;
	}
}

Vector3 CASPlayerLaserBeam::GetEndPoint(Node *node)
{
	auto input = node->GetSubsystem<Input>();
	auto scene = node->GetScene();
	auto octree = scene->GetComponent<Octree>();

	auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
	IntVector2 mousepos;
	if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
	else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
	Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

	Vector3 groundpos(ground.x_, 0.0, ground.y_);
	auto turretnode = node->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("Turret")->node_;
	auto turretpos = turretnode->GetWorldPosition();

	// Do a raycast from turret pos to ground pos to find actual endpoint
	Ray ray(turretpos, groundpos-turretpos);
	PODVector<RayQueryResult> result;
	result.Clear();
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, 100.0f, DRAWABLE_GEOMETRY);
	octree->Raycast(query);

	Vector3 endpos = groundpos;
	for(unsigned int i=0; i<result.Size(); ++i)
	{
		if(result[i].distance_>=0 && TopLevelNode(result[i].drawable_, scene)->GetVar("world").GetBool())
		{
			endpos=ray.origin_+ray.direction_*result[i].distance_;
			break;
		}
	}
	return endpos;
}

CombatActionState *CASPlayerLaserBeam::Update(CombatController *actor, float dt)
{
	auto node = actor->GetNode();
	auto input = node->GetSubsystem<Input>();
	auto scene = node->GetScene();
	auto octree = scene->GetComponent<Octree>();
	auto pd=actor->GetSubsystem<PlayerData>();

	auto vitals=node->GetComponent<PlayerVitals>();
	if(vitals)
	{
		if(vitals->GetCurrentLife()<=0)
		{
			return actor->GetState<CASPlayerDead>();
		}
	}

	StatSetCollection ssc=pd->GetStatSetCollection(EqTurret, "LaserBeam");
	StatSet phases;
	float interval=GetStatValue(ssc, "LaserBeamInterval");

	float p=(float)((int)(phase_ / interval));
	phases.AddMod("LaserBeamPhase", StatModifier::FLAT, std::to_string(p));
	ssc.push_back(&phases);
	phase_ += dt;

	if(input->GetMouseButtonDown(MOUSEB_RIGHT))
	{
		double energy=pd->GetEnergy();
		auto stats=pd->GetStatSetCollection(EqTurret, "LaserBeam");
		double cost=GetStatValue(stats, "EnergyCost");

		if(pd->GetAttack() != PALaserBeam || cost*dt>energy)
		{
			if(input->GetMouseButtonDown(MOUSEB_LEFT))
			{
				return actor->GetState<CASPlayerMove>();
			}
			else return actor->GetState<CASPlayerIdle>();
		}
		pd->SetEnergy(energy-cost*dt);

		Vector3 endpos = GetEndPoint(node);

		Vector3 delta = endpos - lastendpos_;
		Vector3 normdelta = delta.Normalized();
		float length=delta.Length();
		float trackspeed=GetStatValue(ssc, "LaserBeamTrackSpeed")*dt;
		length=std::min(trackspeed, length);

		endpos = lastendpos_ + normdelta*length;
		lastendpos_=endpos;

		if(endburst_) endburst_->SetWorldPosition(endpos);
		if(beam_)
		{
			auto turretnode = node->GetComponent<AnimatedModel>()->GetSkeleton().GetBone("Turret")->node_;
			auto turretpos = turretnode->GetWorldPosition();
			beam_->SetWorldPosition(turretpos);
			auto zaxis = endpos - turretpos;
			zaxis.Normalize();

			Vector3 yaxis(0,1,0);
			Vector3 xaxis = yaxis.CrossProduct(zaxis);

			yaxis = zaxis.CrossProduct(xaxis);

			beam_->SetRotation(Quaternion(xaxis, yaxis, zaxis));
			beam_->SetScale(Vector3(1,1,(endpos-turretpos).Length()*0.5));
		}

		if(timetopulse_ <= 0.0)
		{
			PODVector<Node *> dudes;
			node->GetScene()->GetChildrenWithComponent<EnemyVitals>(dudes, false);

			for(auto i=dudes.Begin(); i!=dudes.End(); ++i)
			{
				Vector3 pos=(*i)->GetWorldPosition();
				Vector3 delta=endpos-pos;
				if(delta.Length() < 12.0)
				{
					auto myvitals = node->GetComponent<PlayerVitals>();
					auto vtls = (*i)->GetComponent<EnemyVitals>();
					if(vtls && myvitals)
					{
						DamageValueList dmg=BuildDamageList(ssc);

						vtls->ApplyDamageList(myvitals,ssc,dmg);
					}
				}
			}

			timetopulse_=interval;
		}
		else
		{
			timetopulse_ -= dt;
		}

		return nullptr;
	}
	else
	{
		if(input->GetMouseButtonDown(MOUSEB_LEFT)) return actor->GetState<CASPlayerMove>();
		return actor->GetState<CASPlayerIdle>();
	}
}

CASPlayerCharge::CASPlayerCharge(Context *context) : CASPlayerBase(context)
{
}

void CASPlayerCharge::Start(CombatController *actor)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();
	auto pd=actor->GetSubsystem<PlayerData>();

	if(node)
	{
		auto stats=pd->GetStatSetCollection(EqDriveSystem, "Charge");
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Play(actor->GetAnimPath() + "/Models/Action.ani", 0, false, 0.1f);
			float attackspeed=std::max(0.01, GetStatValue(stats, "AttackSpeed"));
			ac->SetSpeed(actor->GetAnimPath() + "/Models/Action.ani", attackspeed);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			//ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_HIGH);
			ca->SetEnabled(false);
		}

		auto cam=node->GetScene()->GetChild("Camera")->GetComponent<ThirdPersonCamera>();
		IntVector2 mousepos;
		if(input->IsMouseVisible()) mousepos=input->GetMousePosition();
		else mousepos=node->GetSubsystem<UI>()->GetCursorPosition();
		Vector2 ground=cam->GetScreenGround(mousepos.x_,mousepos.y_);

		auto nav=node->GetScene()->GetComponent<DynamicNavigationMesh>();

		Vector3 gnd(ground.x_, 0.0f, ground.y_);
		dir_=gnd-node->GetWorldPosition();
		dir_.Normalize();
		float chargedist=GetStatValue(stats, "ChargeDistance");
		Vector3 end=node->GetWorldPosition()+dir_*chargedist;
		timetodest_ = GetStatValue(stats, "WarpTime");
		counter_=timetodest_;

		dest_=nav->Raycast(node->GetWorldPosition(), end, Vector3(10,10,10));
		charging_=false;
	}
}

void CASPlayerCharge::End(CombatController *actor)
{
	auto node=actor->GetNode();

	if(node)
	{
		auto ac=node->GetComponent<AnimationController>();
		if(ac)
		{
			ac->Stop(actor->GetAnimPath() + "/Models/Action.ani", 0.1f);
		}

		auto ca=node->GetComponent<CrowdAgent>();
		if(ca)
		{
			ca->SetEnabled(true);
			ca->SetNavigationPushiness(NAVIGATIONPUSHINESS_MEDIUM);
			ca->SetTargetPosition(node->GetWorldPosition());
		}
	}
}

CombatActionState *CASPlayerCharge::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();

	auto ac=node->GetComponent<AnimationController>();
	/*if(ac)
	{
		if(ac->IsAtEnd(actor->GetAnimPath() + "/Models/Action.ani"))
		{
			//return actor->GetState<CASEnemyIdle>();
			if(input->GetMouseButtonDown(MOUSEB_LEFT)) return actor->GetState<CASPlayerMove>();
			return actor->GetState<CASPlayerIdle>();
		}
	}*/

	if(charging_)
	{
		if(counter_>0.0f)
		{
			counter_-=dt;
			counter_=std::max(0.0f, counter_);
			Vector3 pos=dest_.Lerp(node->GetWorldPosition(), counter_/timetodest_);
			node->SetWorldPosition(pos);
		}
		else
		{
			if(input->GetMouseButtonDown(MOUSEB_LEFT)) return actor->GetState<CASPlayerMove>();
			return actor->GetState<CASPlayerIdle>();
		}
	}
	else
	{

		auto playerpos = node->GetWorldPosition();
		auto delta = dest_ - node->GetWorldPosition();

		node->SetRotation(node->GetRotation().Slerp(Quaternion(Vector3::FORWARD, delta), 10.0f * dt));
	}
	return nullptr;
}

void CASPlayerCharge::HandleTrigger(CombatController *actor, String animname, unsigned int value)
{
	auto node=actor->GetNode();
	auto pd=actor->GetSubsystem<PlayerData>();
	auto stats=pd->GetStatSetCollection(EqDriveSystem, "Charge");

	double energy=pd->GetEnergy();
	double cost=GetStatValue(stats, "EnergyCost");
	pd->SetEnergy(energy-cost);

	//node->SetWorldPosition(dest_);
	charging_=true;
}
