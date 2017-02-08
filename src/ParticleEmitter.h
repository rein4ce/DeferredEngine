#pragma once
#include "entity.h"

class CParticle;

class CParticleEmitter : public CEntity
{
	TYPE("Particle Emitter");
public:
	CParticleEmitter();
	virtual ~CParticleEmitter();

	virtual void Update(float frametime);
	virtual void Spawn(Vector3 &pos);

private:
	CArray<CParticle*>	particles;
};