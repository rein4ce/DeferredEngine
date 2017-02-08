#pragma once
#include "platform.h"
#include "Entity.h"

class CEnemy : public CEntity
{
	TYPE("Enemy");
public:

	CEnemy();
	virtual ~CEnemy();

	virtual void Update(float frametime);

	int operator()( CEntity *a,  CEntity *b);

	CObject3D *lastWaypoint;
	float lastShot;
};

class CWaypoint : public CEntity
{
	TYPE("Waypoint");
public:

	CWaypoint();	


};