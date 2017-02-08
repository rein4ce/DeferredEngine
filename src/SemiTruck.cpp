#include "platform.h"
#include "SemiTruck.h"
#include "ModelLoader.h"
#include "Game.h"
#include "TestGame.h"


CSemiTruck::CSemiTruck(void)
{
	static CObject3D *model = NULL;

	if (!model)
	{
		model = CModelLoader::Get("models/semitruck/semitruck.obj");			
		model->SetScale(1.5);
	}

	CObject3D *m = model->Copy();
	Add(m);
	CalculateBoundingShape();	

	boundingShape = EBoundingShape::Box;

	serialized = true;
}

CSemiTruck::~CSemiTruck(void)
{
}

