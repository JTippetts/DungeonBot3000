#include "rotationsmoothing.h"

void RotationSmoothing::RegisterObject(Context *context)
{
	context->RegisterFactory<RotationSmoothing>("Custom");
	URHO3D_ACCESSOR_ATTRIBUTE("Rotation Velocity", GetRotationVelocity, SetRotationVelocity, float, 720.0f, AM_DEFAULT);
}

RotationSmoothing::RotationSmoothing(Context *context) : LogicComponent(context), curangle_(0.0f), nextangle_(0.0f), lastuse_(0.0f), lastusedelta_(0.0f)
{
	SetUpdateEventMask(USE_UPDATE);
}

void RotationSmoothing::SetAngle(float a)
{
	while(a<0.0f) a+=360.0f;
	while(a>=360.0f) a-=360.0f;
	nextangle_=a;
}

void RotationSmoothing::Update(float dt)
{
	float delta1 = nextangle_ - curangle_;
	float delta2 = (nextangle_ + 360.0f) - curangle_;
	float delta3 = (nextangle_ - 360.0f) - curangle_;

	float m1=std::abs(delta1);
	float m2=std::abs(delta2);
	float m3=std::abs(delta3);
	float use=std::min(m1, std::min(m2, m3));
	float usedelta=0;

	if(use<0.001f)
	{
		curangle_=nextangle_;
		node_->SetRotation(Quaternion(curangle_,Vector3(0,-1,0)));
		return;
	}

	if(use==m1) usedelta=delta1;
	else if(use==m2) usedelta=delta2;
	else usedelta=delta3;
	lastuse_=use;
	lastusedelta_=usedelta;

	float sign=1.0f;
	if(usedelta<0.0f) sign=-1.0f;

	float step=dt*rotvel_;
	step=std::min(step,use);
	curangle_=curangle_+step*sign;
	while(curangle_>=360.0f) curangle_-=360.0f;
	while(curangle_<0.0f) curangle_+=360.0f;

	node_->SetRotation(Quaternion(curangle_,Vector3(0,-1.0f,0)));
}
