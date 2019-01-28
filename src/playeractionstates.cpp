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

#include "combatactionstates.h"
#include "Components/thirdpersoncamera.h"
#include "Components/combatcontroller.h"
#include "playerdata.h"
#include "Components/vitals.h"

#include "playeractionstates.h"

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


/////// Idle
CASPlayerIdle::CASPlayerIdle(Context *context) : CombatActionState(context)
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

	if(input->GetMouseButtonPress(MOUSEB_RIGHT))
	{
		// Do right button
		//return nullptr;
		return actor->GetState<CASPlayerSpinAttack>();
	}
	else if(input->GetMouseButtonPress(MOUSEB_LEFT))
	{
		// Do walk
		return actor->GetState<CASPlayerMove>();
	}
	else if(input->GetKeyDown(KEY_Q))
	{
		return actor->GetState<CASPlayerLaserBeam>();
	}

	return nullptr;
}

void CASPlayerIdle::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{
	auto node=actor->GetNode();
	auto ca=node->GetComponent<CrowdAgent>();
	if(ca)
	{
		ca->SetTargetPosition(node->GetWorldPosition());
	}
}

////// PlayerMove
CASPlayerMove::CASPlayerMove(Context *context) : CombatActionState(context)
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

	if(input->GetMouseButtonPress(MOUSEB_RIGHT))
	{
		return actor->GetState<CASPlayerSpinAttack>();
	}
	else if(input->GetMouseButtonDown(MOUSEB_LEFT))
	{
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

void CASPlayerMove::HandleAgentReposition(CombatController *actor, Vector3 velocity, float dt)
{

}


////////////// SpinAttack
CASPlayerSpinAttack::CASPlayerSpinAttack(Context *context) : CombatActionState(context)
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
	}
}

CombatActionState *CASPlayerSpinAttack::Update(CombatController *actor, float dt)
{
	auto node=actor->GetNode();
	auto input=actor->GetSubsystem<Input>();

	if(input->GetMouseButtonDown(MOUSEB_RIGHT))
	{
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
					StatSetCollection ssc=pd->GetStatSetCollection(EqNumEquipmentSlots, "SpinAttack");
					DamageValueList dmg=BuildDamageList(ssc);
					vtls->ApplyDamageList(node,ssc,dmg);
				}
			}
		}
	}
}

//////// Friggin laser beams
CASPlayerLaserBeam::CASPlayerLaserBeam(Context *context) : CombatActionState(context)
{
}

void CASPlayerLaserBeam::Start(CombatController *actor)
{
	auto node = actor->GetNode();
	auto ac = node->GetComponent<AnimationController>();
	auto scene = node->GetScene();
	if(ac)
	{
		ac->StopAll(0.1f);
		ac->Play(actor->GetAnimPath() + "/Models/Idle.ani", 0, true, 0.1f);
	}

	startburst_ = scene->CreateChild();
	endburst_ = scene->CreateChild();
	beam_ = scene->CreateChild();

	auto cache = GetSubsystem<ResourceCache>();
	auto emitter = startburst_->CreateComponent<ParticleEmitter>();
	auto pe = cache->GetResource<ParticleEffect>("Effects/inferno_blaze_particle.xml");
	if(pe)
	{
		emitter->SetEffect(pe);
	}
	emitter = endburst_->CreateComponent<ParticleEmitter>();
	if(pe)
	{
		emitter->SetEffect(pe);
	}

	auto lnode = endburst_->CreateChild();
	lnode->SetPosition(Vector3(0,8,0));
	auto ll = lnode->CreateComponent<Light>();
	ll->SetLightType(LIGHT_POINT);
	ll->SetRange(16);
	ll->SetEnabled(true);
	ll->SetColor(Color(1.5,1.2,1));

	auto beambb = beam_->CreateComponent<StaticModel>();
	if(beambb)
	{
		beambb->SetMaterial(cache->GetResource<Material>("Materials/white.xml"));
		beambb->SetModel(cache->GetResource<Model>("Effects/Beam.mdl"));
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

	startburst_->Remove();
	endburst_->Remove();
	beam_->Remove();
}

CombatActionState *CASPlayerLaserBeam::Update(CombatController *actor, float dt)
{
	auto node = actor->GetNode();
	auto input = node->GetSubsystem<Input>();
	auto scene = node->GetScene();
	auto octree = scene->GetComponent<Octree>();

	if(input->GetKeyDown(KEY_Q))
	{
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
		if(result.Size()==0) return actor->GetState<CASPlayerIdle>();



		Vector3 endpos = groundpos;
		for(unsigned int i=0; i<result.Size(); ++i)
		{
			if(result[i].distance_>=0 && TopLevelNode(result[i].drawable_, scene)->GetVar("world").GetBool())
			{
				endpos=ray.origin_+ray.direction_*result[i].distance_;
				break;
			}
		}

		if(startburst_) startburst_->SetWorldPosition(turretpos);
		if(endburst_) endburst_->SetWorldPosition(endpos);
		if(beam_)
		{
			beam_->SetWorldPosition(turretpos);
			//beam_->SetDirection(endpos-turretpos);
			auto zaxis = endpos - turretpos;
			zaxis.Normalize();

			Vector3 yaxis(0,1,0);
			Vector3 xaxis = yaxis.CrossProduct(zaxis);

			yaxis = zaxis.CrossProduct(xaxis);

			beam_->SetRotation(Quaternion(xaxis, yaxis, zaxis));
			beam_->SetScale(Vector3(1,1,(endpos-turretpos).Length()*0.5));
		}

		return nullptr;
	}
	else
	{
		return actor->GetState<CASPlayerIdle>();
	}
}
