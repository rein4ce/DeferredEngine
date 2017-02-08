#pragma once
#include "platform.h"
#include "entity.h"

class CSpawnPoint :	public CEntity
{
	TYPE("SpawnPoint");
public:
	CSpawnPoint(void);
	virtual ~CSpawnPoint(void);
};
