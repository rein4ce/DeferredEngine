#include "platform.h"
#include "player.h"
#include "Entity.h"
#include "Mesh.h"
#include "Tileset.h"
#include "Resources.h"
#include "Material.h"
#include "Utils.h"
#include "Geometries.h"
#include "Engine.h"


CTexture*	CPlayerEntity::pTexture = NULL;
CMaterial*	CPlayerEntity::pMaterial = NULL;

//////////////////////////////////////////////////////////////////////////
CPlayerEntity::CPlayerEntity() : CEntity()
{
	// initialize static resources
	if (!pTexture)
	{
		pTexture = new CTexture("textures/player3.png");
		pMaterial = new CMaterial();
		pMaterial->features = EShaderFeature::TEXTURE | EShaderFeature::LIGHT;
		pMaterial->pTexture = pTexture;
	}

	// create body parts
	head = new CMesh( new CCubeGeometry(1,1,1));
	torso = new CMesh( new CCubeGeometry(1,1.25f,0.5f));
	larm = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
	rarm = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
	lleg = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
	rleg = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
	helmet = new CMesh( new CCubeGeometry(1,1,1));
	helmet->SetScale(1.1f);

	helmet->geometry->materials.AddToHead(pMaterial);
	head ->geometry->materials.AddToHead(pMaterial);
	torso->geometry->materials.AddToHead(pMaterial);
	larm ->geometry->materials.AddToHead(pMaterial);
	rarm ->geometry->materials.AddToHead(pMaterial);
	lleg ->geometry->materials.AddToHead(pMaterial);
	rleg ->geometry->materials.AddToHead(pMaterial);

	head->Add(helmet);

	for (int i=0; i<head->geometry->vertices.Size(); i++) head->geometry->vertices[i].y += 0.5f;
	for (int i=0; i<helmet->geometry->vertices.Size(); i++) helmet->geometry->vertices[i].y += 0.5f;
	for (int i=0; i<larm->geometry->vertices.Size(); i++) { larm->geometry->vertices[i].x += 0.25f; larm->geometry->vertices[i].y -= 1.25f/2.0f; }
	for (int i=0; i<rarm->geometry->vertices.Size(); i++) { rarm->geometry->vertices[i].x -= 0.25f; rarm->geometry->vertices[i].y -= 1.25f/2.0f; }
	for (int i=0; i<lleg->geometry->vertices.Size(); i++) { lleg->geometry->vertices[i].y -= 1.25f/2.0f; }
	for (int i=0; i<rleg->geometry->vertices.Size(); i++) { rleg->geometry->vertices[i].y -= 1.25f/2.0f; }

	TextureBlock(head,  2,2,2,2, 6,2,2,2, 4,2,2,2, 0,2,2,2, 2,0,2,2, 4,0,2,2);
	TextureBlock(torso, 5,5,2,3, 8,5,2,3, 7,5,1,3, 4,5,1,3, 5,4,2,1, 7,4,2,1);
	TextureBlock(larm,  11,5,1,3, 13,5,1,3, 12,5,1,3, 10,5,1,3, 11,4,1,1, 12,4,1,1);
	TextureBlock(rarm,  11,5,1,3, 13,5,1,3, 12,5,1,3, 10,5,1,3, 11,4,1,1, 12,4,1,1);
	TextureBlock(lleg,  1,5,1,3, 3,5,1,3, 2,5,1,3, 0,5,1,3, 1,4,1,1, 2,4,1,1);
	TextureBlock(rleg,  1,5,1,3, 3,5,1,3, 2,5,1,3, 0,5,1,3, 1,4,1,1, 2,4,1,1);
	TextureBlock(helmet,  10,2,2,2, 14,2,2,2, 12,2,2,2, 8,2,2,2, 10,0,2,2, 12,0,2,2);

	pMesh = new CMesh();

	pMesh->Add(head);
	pMesh->Add(torso);
	pMesh->Add(larm);
	pMesh->Add(rarm);
	pMesh->Add(lleg);
	pMesh->Add(rleg);

	head->SetPosition(0,1.25f/2.0f,0);
	larm->SetPosition(0.5f,1.25f/2.0f,0);
	rarm->SetPosition(-0.5f,1.25f/2.0f,0);
	lleg->SetPosition(0.26f,-1.25f/2.0f,0);
	rleg->SetPosition(-0.26f,-1.25f/2.0f,0);
	
	pMesh->SetScale(0.5f);

	bbox = SBBox(Vector3(-0.25f, -0.25f, -0.25f), Vector3(0.25f, 0.25f, 0.25f));
}

CPlayerEntity::~CPlayerEntity()
{	
}

void CPlayerEntity::Update(float frametime)
{
	float realtime = gEngine.realtime;

	head->SetRotation(sin(realtime)*10.0f, sin(realtime)*10.0f, 0);
	lleg->SetRotation(sin(4*realtime)*30.0f,0,0);
	rleg->SetRotation(cos(4*realtime+M_PI/2.0f)*30.0f,0,0);
	larm->SetRotation(sin(2*realtime)*50.0f,0,sin(2*realtime)*30.0f);
	rarm->SetRotation(cos(2*realtime)*50.0f,0,cos(2*realtime)*30.0f);
}