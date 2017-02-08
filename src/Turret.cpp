#include "platform.h"
#include "Turret.h"
#include "Mesh.h"
#include "Geometry.h"
#include "Geometries.h"
#include "VGUI.h"
#include "TestGame.h"
#include "Physics.h"
#include "CVar.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "ParticleSystem.h"
#include "Renderer.h"

CVar cv_turretRange("turretrange", "25", FCVAR_ARCHIVE);

CTurret::CTurret()
{
	CObject3D *model = CModelLoader::Get("models/turret/turret2.dae")->Copy();
	pBodyMesh = new CMesh();
	pBodyMesh->Add(model);	
	model->SetRotationX(+90);
	model->SetRotationY(+180);
	model->Move(1.2f,-0.5f,0);

	model->SetScale(0.01f);
	pTurretMesh = model->children[0]->children[0]->children[21];
	pTurretMesh->color = RED;

	pBodyMesh->SetPosition(0,0.3f,0);
	
	pTurretMesh->boundingShape = EBoundingShape::Sphere;
	pTurretMesh->frustumCulled = false;

	serialized = true;
	destructible = true;
	pTurretMesh->noHitTest = true;

	this->Add(pBodyMesh);
	nextShoot = 0;
}

CTurret::~CTurret()
{

}

CVar cv_fireFactor("fire", "5", FCVAR_ARCHIVE);

void CTurret::Update( float frametime )
{
	CEntity::Update(frametime);

	
	if (hp <= 0) 
	{
		((CTestGame*)pGame)->CreateExplosion(GetWorldPosition());		
		deleteMe = true;
		return;
	}

	
	if (!pTarget) pTarget = (CEntity*)((CTestGame*)pGame)->pCar;
	
	if (pTarget && pTarget->DistanceTo(this) < 60)
	{
		Vector3 carPos = pTarget->GetWorldPosition();

		LookAt(carPos);
		SetRotationX(0);
		SetRotationZ(0);

		Vector3 dir = (carPos - this->GetWorldPosition());
		float distance = sqrt(dir.x*dir.x + dir.z*dir.z);
		pTurretMesh->SetDirection(Vector3(0, -dir.y, distance).Normalize());	
		pTurretMesh->Rotate(-90,0,0);
		
		if (pTarget->DistanceTo(this) < cv_turretRange.GetFloat())
		{
			if (nextShoot == 0) nextShoot = pGame->realtime + frand(0.25,0.5);
			if (pGame->realtime > nextShoot)
			{
				nextShoot = pGame->realtime  + frand(0.25,0.5);
				gEffects.Create("fire", pTurretMesh->GetWorldVector(Vector3(10,10,-110)) );
				gEffects.Create("fire", pTurretMesh->GetWorldVector(Vector3(40,10,-110)) );

				((CTestGame*)pGame)->ShootBullet(pTurretMesh->GetWorldVector(Vector3(10,10,-140)), -pTurretMesh->forward, true);
			}
		}
	}
}