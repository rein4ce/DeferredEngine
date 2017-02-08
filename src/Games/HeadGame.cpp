#include "HeadGame.h"
#include "Scene.h"
#include "Controllers.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "Geometry.h"
#include "Material.h"
#include "Renderer.h"
#include "Light.h"
#include "Mesh.h"
#include "Entity.h"
#include "Input.h"
#include "Utils.h"
#include "Engine.h"
#include "Geometries\PlaneGeometry.h"

void CHeadGame::Init()
{
	CGame::Init();

	pScene->ambientColor = SRGBA(0x44, 0x33, 0x33, 255);
	gRenderer.clearColor = BLACK;
	pScene->fog = new SFog(SRGBA(255, 245, 233, 130), 450, 1200);
		
	auto mesh = CModelLoader::Get("models/LeePerrySmith/LeePerrySmith.obj");		

	CGeometry *mergedGeom = new CGeometry();

	dirLight = new CDirectionalLight(Vector3(-1, -0.85, 1.5), SRGBA(0xFF, 0xDD, 0xCC, 5));
	dirLight->intensity = 1;
	dirLight->castShadow = true;
	dirLight->width = 8;
	dirLight->height = 8;
	dirLight->shadowNear = 0.01;
	dirLight->shadowFar = 50.0f;
	dirLight->position.Set(1, 0.85, -1.5);
	dirLight->position *= 2;
	pScene->Add(dirLight);

	
	auto backLight = new CDirectionalLight(Vector3(1, -0.75, 0.5), SRGBA(0xCC, 0xCC, 0xFF, 5));
	backLight->intensity = 0.5;
	pScene->Add(backLight);

	for (int i = 0; i < mesh->children.Size(); i++)
	{
		auto child = mesh->children[i];
		auto geom = child->children[0]->geometry;
		geom->ComputeVertexNormals();

		int vertexOffset = mergedGeom->vertices.Size();
		for (int f = 0; f < geom->faces.Size(); f++)
		{
			auto &face = geom->faces[f];
			face.a += vertexOffset;
			face.b += vertexOffset;
			face.c += vertexOffset;
		}

		mergedGeom->vertices.AddVectorToTail(geom->vertices);
		mergedGeom->faces.AddVectorToTail(geom->faces);
	}	

	mergedGeom->useVertexNormals = true;
	mergedGeom->MergeVertices();
	mergedGeom->ComputeFaceNormals();
	mergedGeom->ComputeVertexNormals();
	mergedGeom->ComputeBoundingShape();
	mergedGeom->ComputeCentroids();

	// Default shader is GBuffer shader, which requires position/normal/texcoord
	auto diffuseMaterial = new CMaterial();
	diffuseMaterial->features = EShaderFeature::COLOR | EShaderFeature::TEXTURE | EShaderFeature::LIGHT;	// light is required for normals
	diffuseMaterial->pTexture = CTexture::Get("models/LeePerrySmith/Map-COL.jpg");
	diffuseMaterial->pSpecularTexture = CTexture::Get("models/LeePerrySmith/Map-Spec.jpg");
	diffuseMaterial->pNormalTexture = CTexture::Get("models/LeePerrySmith/Infinite-Level_02_Tangent_SmoothUV.jpg");
	diffuseMaterial->useAlphaSpecular = false;
	diffuseMaterial->specular = 0.5;
	diffuseMaterial->specularPower = 0.5;
	mergedGeom->materials.AddToTail(diffuseMaterial);	

	auto entity = new CEntity();
	entity->Add(new CMesh(mergedGeom, nullptr));
	entity->SetScale(10, 10, 10);
	entity->SetPosition(0, 0.75f, 0);
	entity->castShadow = true;
	entity->receiveShadow = true;
	AddEntity(entity);

	auto ground = new CMesh(new CPlaneGeometry(10, 10));
	pScene->Add(ground);
	ground->SetPosition(-5, -2, -5);
	ground->receiveShadow = true;
	for (int i = 0; i < 3; i++)
	{
		ground->geometry->faces[0].texcoord[i] *= 4.0f;
		ground->geometry->faces[1].texcoord[i] *= 4.0f;
	}
	ground->material = new CMaterial("textures/512.png");
	ground->material->specular = 0.2;

	auto shutters = new CMesh(new CPlaneGeometry(4, 4));
	pScene->Add(shutters);
	for (int i = 0; i < shutters->geometry->vertices.Size(); i++)
	{
		shutters->geometry->vertices[i] += Vector3(-2, 0, -2);
	}
	shutters->material = new CMaterial("textures/shutters2.png");
	shutters->material->transparent = true;
	shutters->material->doubleSided = true;
	shutters->material->alphaTest = 0.1;
	shutters->position = dirLight->position;
	shutters->position *= 0.8f;	
	shutters->LookAt(Vector3());
	shutters->Rotate(90, 0, 0);
	shutters->castShadow = true;

	pCameraController = new COrbitController(pCamera, mesh, Vector3(0, 0, 0), 5.f);
	gInput.SetCenterCursor(true);
}

void CHeadGame::Update(float frametime, float realtime)
{
	CGame::Update(frametime, realtime);
}

void CHeadGame::Render(float frametime, float realtime)
{
	pCamera->SetViewOffset(gEngine.GetWidth(), gEngine.GetHeight());
	CGame::Render(frametime, realtime);
}