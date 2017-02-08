#pragma once
#include "entity.h"

class CLight;

class CRocket :	public CEntity
{
	TYPE("Rocket");
public:
	CRocket(void);
	virtual ~CRocket(void);

	virtual void Update(float frametime);
	
public:
	void Explode();

	bool		followTarget;
	Vector3		targetPos;
	CLight		*light;
};
