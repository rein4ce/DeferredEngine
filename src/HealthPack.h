#pragma once
#include "platform.h"
#include "Entity.h"

class CLight;

class CHealthPack : public CEntity
{
	TYPE("HealthPack");
public:
	CHealthPack();
		
	virtual void Spawn(Vector3 &pos);
	virtual void Update(float frametime);

public:
	bool		fadeOut;
	float		dieTime;

	CLight		*pLight;
};