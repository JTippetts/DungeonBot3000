#include "projectile.h"

void Projectile::RegisterObject(Context *context)
{
	context->RegisterFactory<Projectile>();
}

Projectile::Projectile(Context *context) : LogicComponent(context)
{
}

void Projectile::Setup(const Vector3 &start, const Vector3 &end, const float speed, const float archeight)
{
	start_=start;
	end_=end;
	Vector3 delta=end_-start_;
	duration_=delta.Length() / speed;
	ttl_=duration_;
	archeight_=archeight;
	node_->SetWorldPosition(start_);
}

void Projectile::Update(float dt)
{
	static StringHash TriggerPayload("TriggerPayload");

	ttl_ -= dt;
	if(ttl_ <= 0.0f)
	{
		VariantMap vm;
		node_->SendEvent(TriggerPayload, vm);
		node_->Remove();
		return;
	}

	float t = 1.0f-(ttl_ / duration_);
	float at = t * 2.0f - 1.0f;
	at = 1.0f-at*at;
	Vector3 pos = start_.Lerp(end_, t);
	pos.y_ += at*archeight_;

	node_->SetWorldPosition(pos);
}
