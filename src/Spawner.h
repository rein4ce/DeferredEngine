#pragma once
#include "platform.h"
#include "Entity.h"

class CSpawner : public CEntity
{
	TYPE("Spawner");
public:

	CSpawner();

	virtual void Update(float frametime);

public:
	string		entityType;				// type of spawned entities
	float		interval;				// how often spawn
	float		variance;				// variation of the interval
	int			maxCount;				// max spawned objects
	int			maxSpawnedTotal;		// how many spawned in total is allowed

	CArray<CEntity*>	spawned;		// keep track of the spawned objects

private:
	float		nextSpawn;
	int			spawnedCount;
};