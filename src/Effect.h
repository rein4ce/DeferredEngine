#pragma once
#include "platform.h"
#include "Entity.h"

class CParticleEffect;

class CEffect : public CEntity
{
	TYPE("Effect");
public:
	CEffect();

	virtual void Update(float frametime);

public:
	CParticleEffect		*pEffect;				// effect being created

private:
	float				timeCreated;
	float				prevTime;
	int					spawnedCount;			// how many effects have been spawned
};