#include "platform.h"
#include "Utils.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"
#include "Resources.h"
#include "Game.h"
#include "Geometries.h"


CParticleEmitter::CParticleEmitter()
{
	this->geometry = new CCubeGeometry(0.5f, 0.5f, 0.5f);
	this->color = YELLOW;
	CalculateBoundingShape();

	serialized = true;
	noHitTest = true;
	editorOnly = true;
}

CParticleEmitter::~CParticleEmitter()
{
	for (int i=0; i<particles.Size(); i++)
		if (particles[i])
			particles[i]->deleteMe = true;
	particles.Purge();
}

void CParticleEmitter::Update(float frametime)
{
	CEntity::Update(frametime);
}

void CParticleEmitter::Spawn( Vector3 &pos )
{
	CEntity::Spawn(pos);

	for (int i=0; i<5; i++)
	{
		CParticle *p = new CParticle(CTexture::Get("particles/minedust.dds"));
		int size = frand(7,10);
		p->size = Vector2(size, size);
		p->rotation = frand(-90, 90);
		p->position = pos + Vector3(frand(-5,5), frand(-2,0), frand(-5,5));
		p->rotationVel = 10;
		p->depthWrite = false;
		p->color = SRGBA(242,215,150,60);

		pGame->particles->Add(p);
		particles.AddToTail(p);
	}
}