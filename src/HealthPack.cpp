#include "platform.h"
#include "HealthPack.h"
#include "Geometries\CubeGeometry.h"
#include "Material.h"
#include "Resources.h"
#include "Physics.h"
#include "Game.h"
#include "TestGame.h"
#include "Car.h"
#include "..\NewtonWin-2.30\sdk\Newton.h"
#include "Light.h"

CHealthPack::CHealthPack()
{	
	static CMaterial *mat = new CMaterial("textures/healthpack.png");
	this->geometry = new CCubeGeometry(0.5f, 0.5f, 0.5f);
	this->geometry->materials.Add(mat);	
	this->CalculateBoundingShape();
	fadeOut = false;
	pLight = new CPointLight(Vector3());
	pLight->color = RED;
	this->Add(pLight);
}

void CHealthPack::Spawn( Vector3 &pos )
{
	CEntity::Spawn(pos);
	pBody = CPhysics::CreateBox(pGame->pWorld, this, 0.5f, 0.5f, 0.5f, 100);	
	NewtonBodySetForceAndTorqueCallback(pBody, BoxGravityCallback);
}

void CHealthPack::Update( float frametime )
{
	CEntity::Update(frametime);

	if (fadeOut)
	{	
		DeleteBody();
		if (pGame->realtime > dieTime)
		{
			deleteMe = true;			
		}
		else
		{
			if (this->scale.x > 0)
			{
				this->SetScale(max(0,this->scale.x - frametime));				
			}
			this->opacity -= frametime;
			this->opacity = max(0, opacity);			
			this->MoveTo(((CTestGame*)pGame)->pCar->GetWorldPosition(), frametime * 2);
		}
	}
	else
	{
		if (DistanceTo(((CTestGame*)pGame)->pCar->GetWorldPosition()) < 2 && ((CTestGame*)pGame)->hp < 100)
		{
			fadeOut = true;
			dieTime = pGame->realtime + 1;
			((CTestGame*)pGame)->hp = 100;
			((CTestGame*)pGame)->healthPacksNum--;
		}
	}	
}