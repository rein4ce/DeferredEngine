#include "platform.h"
#include "Spawner.h"
#include "Geometries\CubeGeometry.h"
#include "Engine.h"
#include "Game.h"
#include "ConsolePanel.h"

CSpawner::CSpawner()
{
	geometry = new CCubeGeometry(2,1,2);
	color = MAGENTA;
	entityType = "Enemy";
	interval = 2;
	variance = 1.5f;
	maxCount = 5;
	nextSpawn = 0;
	spawnedCount = 0;
	maxSpawnedTotal = 0;

	serialized = true;
	noHitTest = true;
	editorOnly = true;
}

void CSpawner::Update( float frametime )
{
	CEntity::Update(frametime);
	if (nextSpawn == 0) nextSpawn = gEngine.realtime + interval + frand(-variance, +variance);

	for (int i=0; i<spawned.Size(); i++)
	{
		if (spawned[i]->deleteMe)
		{
			spawned.RemoveAt(i--);
		}
	}

	if (spawned.Size() < maxCount && gEngine.realtime > nextSpawn && (spawnedCount < maxSpawnedTotal || maxSpawnedTotal == 0))
	{		
		nextSpawn = 0;

		CEntity *ent = CEntity::Create(entityType);
		ent->SetPosition(GetWorldPosition());
		spawned.Add(ent);
		pGame->AddEntity(ent);

		spawnedCount++;
	}
}
