#include "platform.h"
#include "Game.h"
#include "CVar.h"
#include "Level.h"
#include "Scene.h"
#include "Camera.h"
#include "Tileset.h"
#include "Controllers.h"
#include "Engine.h"
#include "Renderer.h"
#include "VGUI.h"
#include "Sprite.h"
#include "ModelLoader.h"
#include "Level.h"
#include "Tileset.h"
#include "Collision.h"
#include "Primitives.h"
#include "Movement.h"
#include "Tileset.h"
#include "Game.h"
#include "Player.h"
#include "Resources.h"
#include "Material.h"
#include "Entity.h"
#include "ParticleSystem.h"
#include "Terrain.h"
#include <Newton.h>
#include "Level.h"
#include "ConsolePanel.h"
#include "FrameGraph.h"
#include "Serialize.h"
#include "Editor.h"

#define TEMP_SAVE_FILE		"level.map"

CVar cv_gravity = CVar("gravity", "120");
CVar cv_editor = CVar("editor", "0");
CVar cv_splash("splash", "1", FCVAR_ARCHIVE);

CVar cv_fly("fly", "0", FCVAR_ARCHIVE);
CVar cv_maxammo("maxammo", "50", FCVAR_ARCHIVE);
CVar cv_reloadtime("reloadtime", "1", FCVAR_ARCHIVE);
CVar cv_bulletGravity = CVar("bulletgravity", "0.2", FCVAR_ARCHIVE);
CVar cv_bulletSpeed = CVar("bulletspeed", "100.0", FCVAR_ARCHIVE);
CVar cv_bulletScatter = CVar("bulletscatter", "0.02", FCVAR_ARCHIVE);

int CGame::nextEntityId = 0;


//////////////////////////////////////////////////////////////////////////
CGame::CGame()
{	
	pScene = NULL;
	pCamera = NULL;
	pCameraController = NULL;
	pWorld = NULL;
	particles = NULL;
	pPlayer = NULL;
	pLevel = NULL;
	realtime = frametime = 0;
}

CGame::~CGame()
{
	CONSOLE("~CGame()");
	StopsExecution();
	TerminateTask();
	Reset();	
}

//////////////////////////////////////////////////////////////////////////
CEntity* CGame::GetEntityById( int id )
{
	for (int i=0; i<entities.Size(); i++)
		if (entities[i]->id == id) return entities[i];
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CGame::AddEntity( CEntity *entity )
{
	Assert(entity && "Adding a NULL entity");
	
	entity->id = nextEntityId++;
	entities.Add(entity);

	entity->Spawn(entity->position);

	Debug("Entity \"%s\" added, id %d", entity->GetType().c_str(), entity->id);
	pScene->Add(entity);	
}

//////////////////////////////////////////////////////////////////////////
void CGame::RemoveEntity( CEntity *entity )
{
	Assert(entity && "Removing a NULL entity");
	Assert(entity->id >= 0 && "Entity has not been added to the list");
	RemoveEntity(entity->id);
}

//////////////////////////////////////////////////////////////////////////
void CGame::RemoveEntity(int id)
{
	for (int i=0; i<entities.Size(); i++)
		if (entities[i]->id == id) 
		{
			pScene->Remove(entities[i]);		// mark entity for deletion
			entities.RemoveAt(i);				// remove entity from the update list
			return;
		}
}


//////////////////////////////////////////////////////////////////////////
void CGame::Init()
{
	Reset();

	// create scene
	pScene = new CScene();
	
	pScene->CreateSkybox("clear");

	// create camera
	pCamera = new CPerspectiveCamera(60.0f, gEngine.width, gEngine.height, 0.1f, 1000.0f );
	pCamera->SetPosition(-5,1,0);
	pCamera->SetRotation(20,-80,0);	

	// create player
	pPlayer = new CPlayerEntity();
	AddEntity(pPlayer);
	
	// create physics
	pWorld = NewtonCreate();

	NewtonSetPlatformArchitecture (pWorld, 3);
	NewtonSetSolverModel(pWorld, 1);
	NewtonSetFrictionModel(pWorld, 1); 
	NewtonSetThreadsCount(pWorld, 2);
	NewtonSetMultiThreadSolverOnSingleIsland(pWorld, 1);

	Vector3 min = Vector3(-1000,-10,-1000);
	Vector3 max = Vector3(+1000,+1000,+1000);
	NewtonSetWorldSize(pWorld, &min[0], &max[0]);

	// create particle system
	particles = new CParticleSystem(pScene, 10000);
}

//////////////////////////////////////////////////////////////////////////
void CGame::PostInit()
{
	// start physics thread
	InitPhysicsThread();
}

//////////////////////////////////////////////////////////////////////////
void CGame::InitPhysicsThread()
{
	pPhysicsCallback = PhysicsThreadFn<CGame>(this);
	framesPerSecond = 60;
}

//////////////////////////////////////////////////////////////////////////
void CGame::Reset()
{
	SAFE_DELETE(pCamera);
	SAFE_DELETE(pCameraController);
	SAFE_DELETE(pLevel);
	
	entities.PurgeAndDeleteElements();
	SAFE_DELETE(particles);
	
	if (pWorld)
	{
		NewtonDestroyAllBodies(pWorld);
		NewtonDestroy(pWorld);
		pWorld = NULL;
	}	
}

//////////////////////////////////////////////////////////////////////////
void CGame::Render( float frametime, float realtime )
{
	gRenderer.Render(*pScene, *pCamera);	
}


//////////////////////////////////////////////////////////////////////////
// Update doesn't happen every Rendering frame
//////////////////////////////////////////////////////////////////////////
void CGame::Update(float frametime, float realtime)
{
	this->frametime = frametime;
	this->realtime = realtime;

	gFrameGraph.Begin("Scene Update", GREEN);
	pScene->Update(frametime);
	gFrameGraph.End("Scene Update");

	gFrameGraph.Begin("Entities Update", YELLOW);
	UpdateEntities(frametime, realtime);
	gFrameGraph.End("Entities Update");

	gFrameGraph.Begin("Particles Update", BLACK);
	if (particles) particles->Update(frametime, realtime);
	gFrameGraph.End("Particles Update");
	if (pCameraController) pCameraController->Update(frametime );
	
	gTerrain.Update(frametime);

	if (pLevel)
		pLevel->Update();
}

//////////////////////////////////////////////////////////////////////////
void CGame::UpdateEntities( float frametime, float realtime )
{
	for (int i=0; i<entities.Size(); i++)
	{
		CEntity *ent = entities[i];
		if (!ent) continue;		

		// remove entities marked for deletion
		if (ent->deleteMe)
		{
			Debug("Removing entity %d", i);
			pScene->RemoveObject(ent);			// renderer will delete the object
			entities.RemoveAt(i--);
			continue;
		}

		// update entity logic
		entities[i]->Update(frametime);

		// if entity is outsiude of the world, kill it
		if (abs(entities[i]->position.x) > 1000.0f ||
			abs(entities[i]->position.y) > 1000.0f ||
			abs(entities[i]->position.z) > 1000.0f)
			entities[i]->deleteMe = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CGame::UpdatePhysics(float frametime)
{
	EnterCriticalSection(&renderCS);
	NewtonUpdate(pWorld, frametime);
	LeaveCriticalSection(&renderCS);
}

//////////////////////////////////////////////////////////////////////////
// Shoot a ray through the game world and check for intersections
//////////////////////////////////////////////////////////////////////////
bool CGame::CastRay( Vector3 &start, Vector3 &end, int type, OUT SRayCastResult &result )
{
	bool voxelHit = false, terrainHit = false, objectsHit = false;
	float maxLen = 9999.9f;
	float voxelLen = maxLen, terrainLen = maxLen, objectsLen = maxLen;
	Vector3 voxelPos = end, terrainPos = end, objectsPos = end;
			

	// check collision against voxel level
	if ((type & RC_VOXEL) && pLevel)
	{
		voxelHit = pLevel->CastRay(start, end, OUT result.tilePos, OUT &result.tile, OUT &voxelPos);
		voxelLen = (voxelPos - start).Length();
	}

	// check collision against terrain
	if (type & RC_TERRAIN)
	{
		terrainHit = gTerrain.CastRay(start, end, OUT terrainPos);
		terrainLen = (terrainPos - start).Length();
	}

	// TODO: check collision against other objects
	if (type & RC_OBJECTS)
	{		
		for (int i=0; i<entities.Size(); i++)
		{
			CEntity *ent = entities[i];
			Vector3 pos;
			if (ent->LineBoundingSphereIntersection(start, end, OUT pos))
			{
				objectsHit = true;				
				float dist = (pos - start).Length();
				if (dist < objectsLen)
				{
					objectsLen = dist;
					result.object = ent;
					objectsPos = pos;
				}								
			}
		}
	}

	// now pick the closest one
	float minLength = min(voxelLen, min(terrainLen, objectsLen));
	if (minLength == voxelLen && voxelHit) result.type = RC_VOXEL;
	if (minLength == terrainLen && terrainHit) result.type = RC_TERRAIN;
	if (minLength == objectsLen && objectsHit) result.type = RC_OBJECTS;
	
	result.hitPos = start + (end - start).Normalize() * minLength;

	// check collision against terrain
	return voxelHit || terrainHit || objectsHit;
}


//////////////////////////////////////////////////////////////////////////
// Save the current voxel/entity array into the file
//////////////////////////////////////////////////////////////////////////
void CGame::Save( string filename )
{
	ofstream fs(filename.c_str(), ios::out | ios::binary);
	
	if (pLevel)
		pLevel->Save(fs);				// Save the voxel map
	SaveEntities(fs);				// Save the entities

	fs.close();

	CONSOLE("Game saved");
}

//////////////////////////////////////////////////////////////////////////
bool CGame::Load(string filename)
{
	ifstream fs(filename.c_str(), ios::in | ios::binary);
	
	if (pLevel)
		if (!pLevel->Load(fs)) return false;
	LoadEntities(fs);

	fs.close();

	CONSOLE("Game loaded");
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGame::SaveEntities( ofstream &fs )
{
	int count = 0;
	for (int i=0; i<entities.Size(); i++)
		if (entities[i]->serialized) count++;

	fs.write((char*)&count, sizeof(count));

	Debug("Writing %d entities", count);
	for (int i=0; i<entities.Size(); i++)
	{
		CEntity *e = entities[i];
		if (!e->serialized) continue;
				
		string name = e->GetType();	
		
		CSerialize::Write(fs, &name);	
		CSerialize::Write(fs, &e->id);
		entities[i]->Save(fs);

		int null = -1;
		if (e->pTarget)
			CSerialize::Write(fs, &e->pTarget->id);
		else
			CSerialize::Write(fs, &null);
	}

}

//////////////////////////////////////////////////////////////////////////
void CGame::LoadEntities( ifstream &fs )
{
	int count = 0;
	fs.read((char*)&count, sizeof(count));

	Debug("Loading %d entities", count);

	// Remove all serialized objects before loading
	for (int i=0; i<entities.Size(); i++)
	{
		if (entities[i]->serialized)
		{
			pScene->Remove(entities[i]);
			entities.RemoveAt(i);
			i--;
		}
	}

	map<int, int> targetMap;

	for (int i=0; i<count; i++)
	{		
		string name;
		int len;

		CSerialize::Read(fs, &name);
		
		CEntity *e = CEntity::Create(name);
		CSerialize::Read(fs, &e->id);					// id

		e->Load(fs);

		int targetId;
		fs.read((char*)&targetId, sizeof(e->id));
		if (targetId != -1) targetMap[e->id] = targetId;
		
		e->updateMatrix = true;
		AddEntity(e);
		e->UpdateMatrixWorld(true);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGame::FindEntitiesByType( string name, OUT CArray<CEntity*> &ret )
{
	for (int i=0; i<entities.Size(); i++)
		if (entities[i]->GetType() == name)
			ret.Add(entities[i]);
}

//////////////////////////////////////////////////////////////////////////
void CGame::FindEntitiesByTypeInRange( string name, Vector3 pos, float radius, OUT CArray<CEntity*> &ret )
{
	for (int i=0; i<entities.Size(); i++)
		if (entities[i]->GetType() == name && entities[i]->DistanceTo(pos) <= radius)
			ret.Add(entities[i]);
}

//////////////////////////////////////////////////////////////////////////
CPhysicsThread::CPhysicsThread() : dThread()
{	
	framesPerSecond = 60;
}

//////////////////////////////////////////////////////////////////////////
unsigned CPhysicsThread::RunMyTask()
{
	// calculate frametime
	static double oldTime = GetTime();
	float newTime = GetTime();
	float frametime = newTime - oldTime;
	oldTime = newTime;

	static float physAcc = 0;
	physAcc += frametime;
	if (physAcc > 1.0f/framesPerSecond)
	{
		pPhysicsCallback(physAcc);
		physAcc = 0;
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
double CPhysicsThread::GetTime()
{
	static LARGE_INTEGER PerformanceFrequency;
	static LARGE_INTEGER MSPerformanceFrequency;
	static LARGE_INTEGER ClockStart;

	if( !PerformanceFrequency.QuadPart )
	{
		QueryPerformanceFrequency(&PerformanceFrequency);
		MSPerformanceFrequency.QuadPart = PerformanceFrequency.QuadPart / 1000;
		QueryPerformanceCounter(&ClockStart);
	}
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter( &CurrentTime );
	double fRawSeconds = (double)( CurrentTime.QuadPart - ClockStart.QuadPart ) / (double)(PerformanceFrequency.QuadPart);

	return fRawSeconds;
}
