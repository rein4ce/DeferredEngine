#pragma once
#include "Geometry.h"
#include "Geometries/PlaneGeometry.h"
#include "Geometries/CubeGeometry.h"
#include "Geometries/SphereGeometry.h"
#include "Geometries/CylinderGeometry.h"


class CImage;

class CHeightmapGeometry : public CGeometry
{
public:
	CHeightmapGeometry(float width = 10.0f, float height = 10.0f, int segmentsWidth = 1, int segmentsHeight = 1, float *heightmap = NULL);
	CHeightmapGeometry(CImage *heightmap, float tileWidth = 1, float tileHeight = 1);

	void Create(float width = 10.0f, float height = 10.0f, int segmentsWidth = 1, int segmentsHeight = 1, float *heightmap = NULL);
	void Create(CImage *heightmap, float tileWidth = 1, float tileHeight = 1);

public:
	float		width, height;
	int			segmentsX, segmentsY;
};