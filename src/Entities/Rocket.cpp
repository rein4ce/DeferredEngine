#include "platform.h"
#include "Rocket.h"
#include "Object3D.h"
#include "Geometry.h"
#include "Geometries.h"
#include "ModelLoader.h"
#include "Mesh.h"
#include "Game.h"
#include "ParticleSystem.h"
#include "Resources.h"
#include "Input.h"
#include "EffectsGame.h"
#include "CVar.h"
#include "Level.h"
#include "Camera.h"
#include "TestGame.h"
#include "Light.h"
#include "Object3D.h"

CVar cv_explosionforce = CVar("explosionforce", "15", FCVAR_ARCHIVE);
CVar cv_explosionout = CVar("explosionout", "5", FCVAR_ARCHIVE);


CRocket::CRocket(void)
{
	static bool loaded = false;
	static CObject3D *model = CModelLoader::Get("models/Small_Rocket_B.3ds");
	if (!loaded)
	{
		((CMesh*)model->children[0]->children[0])->Reposition();
		loaded = true;
	}

	CObject3D* mesh = model->Copy();	
	
	mesh->SetScale(0.1);
	mesh->MoveRight(-1.25f);
	mesh->MoveForward(-0.5 + 1);
	mesh->MoveUp(-0.25-0.2f);

	this->Add(mesh);
	boundingShape = EBoundingShape::Sphere;

	followTarget = false;

	light = new CPointLight(Vector3(), YELLOW);
	//light->range = 3.0f;
	this->Add(light);
}

CRocket::~CRocket(void)
{
	
}

//////////////////////////////////////////////////////////////////////////
void CRocket::Update( float frametime )
{
	Vector3 pos = this->GetPosition();			// start position

	CEntity::Update(frametime);
	
	Vector3 end = this->GetPosition();

	if (followTarget)
	{
		this->LookAt(pTarget->GetWorldPosition());
		this->acceleration = forward * 5.0f;
		this->acceleration.y -= 1;
	}
	
	
	// engine
	static CTexture *muzzleTex = CTexture::Get("particles/flamer.dds");
	static CTexture *smokeTex = CTexture::Get("particles/explosion2.dds");
	
	// create particle effect for the engine flame
	{
		CParticle *p = new CParticle(muzzleTex);
		p->size = Vector2(0.7, 0.7);
		p->position = pos + forward*(-0.6);								
		p->lifetime = 0.01;

		float s = frand(1, 1.3);
		p->size = Vector2(s,s);
		p->rotation = frand(0,90);
		p->depthWrite = false;

		p->additive = true;
		pGame->particles->Add(p);
	}

	static float smokeAcc = 0;

	// create smoke trail particles
	smokeAcc += frametime;
	if (smokeAcc > 0.02 )
	{
		CParticle *p = new CParticle(smokeTex);
		p->size = Vector2(0.35, 0.35);
		p->position = this->GetWorldPosition() + forward*(-0.7);			
		p->lifetime = 5;

		p->rotation = frand(0,90);
		p->depthWrite = false;

		p->color = SRGBA(255,255,255,100);
		p->sizeVel = Vector2(0.65,0.65);		
		p->colorChange = SRGBA(0,0,0,0);
		p->colorTime = 5;

		pGame->particles->Add(p);

		smokeAcc = 0;		
	}

	// check if rocket is leaving the world	
	if ((position - pGame->pCamera->GetWorldPosition()).Length() > 500)
	{
		this->deleteMe = true;
		this->Remove(light);
	}

	// check collision of the rocket with the world	
	SRayCastResult rc;
	if (pGame->CastRay(pos, end, RC_VOXEL | RC_TERRAIN, OUT rc))
	{
		if (!(rc.type == RC_OBJECTS && rc.object == (CObject3D*)((CTestGame*)pGame)->pCar)) 
		{
			if (!(rc.type == RC_OBJECTS && rc.object->GetType() == "Rocket"))
			{
				Explode();
				deleteMe = true;

				if (rc.type == RC_VOXEL)
					((CTestGame*)pGame)->ExplodeTiles(rc.tilePos, 1.8f);

				if (rc.type == RC_OBJECTS)
					rc.object->hp -= 75;
			}
		}		
	}
}

//////////////////////////////////////////////////////////////////////////
void CRocket::Explode()
{
	Vector3 pos = GetPosition();
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	int r = 3;

	this->Remove(light);

	CArray<CEntity*> enemies; 
	pGame->FindEntitiesByTypeInRange("Enemy", GetWorldPosition(), 5, OUT enemies);
	for (int i=0; i<enemies.Size(); i++)
		enemies[i]->hp -= 80;

	x -= pLevel->offset.x;
	y -= pLevel->offset.y;
	z -= pLevel->offset.z;
	
	// destroy tiles in the area
	for (int ix = x, xsign = 1, xstep = 1; 
		(ix >= x - r) && (ix <= x + r);
		ix += xsign * xstep, xsign = -xsign, xstep++)
		for (int iy = y, ysign = 1, ystep = 1; 
			(iy >= y - r) && (iy <= y + r);
			iy += ysign * ystep, ysign = -ysign, ystep++)
			for (int iz = z, zsign = 1, zstep = 1; 
				(iz >= z - r) && (iz <= z + r);
				iz += zsign * zstep, zsign = -zsign, zstep++)
			{
				SMapTile *tile = NULL;
				if ((tile = pLevel->GetTile(ix,iy,iz)) == NULL) continue;

				if (tile->type == 0) continue;

				if (frand(0, 1) > 0.8)

				tile->type = 0;

				SMapChunk *chunk = pLevel->GetChunk(
					floor((float)ix/SMapChunk::Size),
					floor((float)iy/SMapChunk::Size),
					floor((float)iz/SMapChunk::Size) );

				chunk->dirtyBody = true;
				chunk->dirty = true;

				SSpawnTile s;
				s.pos = Vector3(ix,iy,iz) + pLevel->offset;
				s.vel = (Vector3(ix+frand(-0.25f,+0.25),iy+frand(-0.25f,+0.25),iz+frand(-0.25f,+0.25)) - pos).Normalize() * cv_explosionout.GetFloat() + (-forward)*cv_explosionforce.GetFloat();
				
				CEffectsGame::pInstance->SpawnTile(s);				
			}
			

	// add particle effect
	static CTexture *expTex = CTexture::Get("particles/flamer.dds");

	CParticle *p = new CParticle(expTex);
	p->position = pos;
	p->size = Vector2(10,10);
	p->sizeVel = Vector2(15,15);
	p->rotationVel = 90;
	p->colorChange = SRGBA(255,255,255,100);
	p->colorTime = 10;
	p->lifetime = 5;
	p->additive = true;
	p->depthWrite = false;
	pGame->particles->Add(p);

	p = new CParticle(expTex);
	p->position = pos;
	p->size = Vector2(8,8);
	p->sizeVel = Vector2(10,10);
	p->rotationVel = 90;
	p->colorChange = SRGBA(22,22,22,100);
	p->colorTime = 5;
	p->lifetime = 5;
	p->additive = true;
	p->depthWrite = false;
	pGame->particles->Add(p);
}
