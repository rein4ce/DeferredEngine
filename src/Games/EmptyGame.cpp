#include "platform.h"
#include "EmptyGame.h"
#include "Scene.h"
#include "Camera.h"
#include "Level.h"
#include "Controllers.h"
#include "Editor.h"
#include "Geometries\CubeGeometry.h"
#include "Material.h"
#include "ModelLoader.h"
#include "Input.h"
#include "Entity.h"
#include "Light.h"
#include "Mesh.h"
#include "Geometries\PlaneGeometry.h"


CEmptyGame::CEmptyGame()
{

}

CEmptyGame::~CEmptyGame()
{

}

void CEmptyGame::Init()
{
	CGame::Init();

	pScene->ambientColor = SRGBA(0x44, 0x33, 0x33, 255);
	gRenderer.clearColor = BLACK;
	pScene->fog = new SFog(SRGBA(255, 245, 233, 130), 450, 1200);

	pCameraController = new CFlyController(pCamera);
	pCamera->SetPosition(4, 3, 4);
	pCamera->LookAt(0, 1, 0);	

	// create an entity
	CEntity *ent = new CEntity();
	
	CObject3D *mesh = CModelLoader::Get("models/city/export.obj");

	auto mat = new CMaterial("models/city/export-tiny-RGB.png");
	mat->specular = 0.2;
	mat->useFiltering = false;

	for (int i = 0; i < mesh->children.Size(); i++)
	{
		auto child = mesh->children[i];
		if (child->children.Size() == 0) continue;
		auto geom = child->children[0]->geometry;
		if (!geom) continue;
		geom->ComputeVertexNormals();

		geom->materials[0] = mat;
	}

	
	ent->Add(mesh);
	ent->position.y = -5.0f;
	ent->SetScale(0.2, 0.2, 0.2);
	AddEntity(ent);

	auto ground = new CMesh(new CPlaneGeometry(100, 100));
	pScene->Add(ground);
	ground->SetPosition(-50, 0, -50);
	ground->receiveShadow = true;
	for (int i = 0; i < 3; i++)
	{
		ground->geometry->faces[0].texcoord[i] *= 40.0f;
		ground->geometry->faces[1].texcoord[i] *= 40.0f;
	}
	ground->material = new CMaterial("textures/512.png");
	ground->material->specular = 0.2;

	for (int x=0; x<10; x++)
		for (int z = 0; z < 10; z++)
		{
			auto pLight = new CPointLight(Vector3((x-5) * 5, frand(5, 15), (z-5) * 5), SRGBA(frand(0, 255), frand(0,255), frand(0,255), 255));
			pScene->Add(pLight);
			pLight->specular = SRGBA(255, 233, 155, 100);
			pLight->range = frand(2,10);
			pLight->intensity = frand(0.1, 1.0);			
			lights.push_back(pLight);
		}

	gInput.SetCenterCursor(true);
}

void CEmptyGame::Reset()
{
	
}

void CEmptyGame::Update( float frametime, float realtime )
{
	CGame::Update(frametime, realtime);
}