#pragma once
#include "platform.h"
#include "Entity.h"

class CMesh;

class CTurret : public CEntity
{
	TYPE("Turret");
public:
	CTurret();
	virtual ~CTurret();

	virtual void Update(float frametime);

public:
	CMesh		*pBodyMesh;
	CObject3D	*pTurretMesh;
	float		nextShoot;
};