#include "platform.h"
#include "Effect.h"
#include "Engine.h"
#include "ParticleSystem.h"
#include "ConsolePanel.h"

CEffect::CEffect()
{
	pEffect = NULL;
	timeCreated = 0;
	prevTime = 0;
	spawnedCount = 0;
}

void CEffect::Update( float frametime )
{
	CEntity::Update(frametime);
	if (timeCreated == 0) timeCreated = gEngine.realtime;

	float time = gEngine.realtime - timeCreated;
	for (int i=0; i<pEffect->particles.Size(); i++)
	{
		SParticleEffectInfo *p = &pEffect->particles[i];
		if (prevTime <= p->time.Get() && time > p->time.Get())
		{
			p->Spawn(this->position);
			spawnedCount++;

			// if we spawned all particles inside this effect, kill it
			if (spawnedCount == pEffect->particles.Size())
			{
				deleteMe = true;				
			}
		}
	}

	prevTime = time;
}