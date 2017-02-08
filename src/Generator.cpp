#include "platform.h"
#include "Generator.h"
#include "ModelLoader.h"
#include "Physics.h"
#include "Game.h"
#include "TestGame.h"
#include "ConsolePanel.h"
#include "Vector.h"

CGenerator::CGenerator(void)
{
	static CObject3D *model = NULL;
	
	if (!model)
	{
		model = CModelLoader::Get("models/generator/models/generator.dae");	
		model->SetScale(0.001f);
		model->SetRotationX(-90);	
		model->Reposition();		
	}

	CObject3D *m = model->Copy();
	Add(m);
	CalculateBoundingShape();	

	boundingShape = EBoundingShape::Box;

	serialized = true;
	destructible = true;

	lastHP = maxhp = hp = 500;	
	killTime = 0;	
}

CGenerator::~CGenerator(void)
{
	CONSOLE("Generator destructor");
}

void CGenerator::Update( float frametime )
{
	CEntity::Update(frametime);

	if (hp < 100 && lastHP >= 100) ((CTestGame*)pGame)->CreateSmallExplosion(spawnPos+Vector3(0,frand(0,5),0)); else
	if (hp < 200 && lastHP >= 200) ((CTestGame*)pGame)->CreateSmallExplosion(spawnPos+Vector3(0,frand(0,5),0)); else
	if (hp < 300 && lastHP >= 300) ((CTestGame*)pGame)->CreateSmallExplosion(spawnPos+Vector3(0,frand(0,5),0)); else
	if (hp < 400 && lastHP >= 400) ((CTestGame*)pGame)->CreateSmallExplosion(spawnPos+Vector3(0,frand(0,5),0));

	if (hp <= 0)
	{
		if (killTime == 0) 
		{
			killTime = pGame->realtime + 3;
			falldir = Vector2(frand(-1,1), frand(-1,1));
			falldir.Normalize();
			boom = false;
			((CTestGame*)pGame)->CreateExplosion(spawnPos+Vector3(0,2,0));
		}

		velocity.y -= frametime/2 ;
		Move(frametime*falldir.y,0,-frametime*falldir.x);
		Rotate(frametime*20*falldir.x,0,frametime*20*falldir.y);
	
		if (pGame->realtime > killTime-2 && !boom)
		{
			((CTestGame*)pGame)->CreateExplosion(spawnPos);
			boom = true;
		}

		if (pGame->realtime > killTime)
		{			
			DeleteBody();
			deleteMe = true;
		}
	}

	lastHP = hp;
}

void CGenerator::Spawn( Vector3 &pos )
{
	CEntity::Spawn(pos);
	spawnPos = pos;
	Vector3 size = Vector3(3,5,3);
}