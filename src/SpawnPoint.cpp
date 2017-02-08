#include "platform.h"
#include "SpawnPoint.h"
#include "Geometries\CubeGeometry.h"

CSpawnPoint::CSpawnPoint(void)
{
	this->geometry = new CCubeGeometry();
	this->color = GREEN;
	CalculateBoundingShape();

	serialized = true;
	editorOnly = true;
	noHitTest = true;
}

CSpawnPoint::~CSpawnPoint(void)
{
}
