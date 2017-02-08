#pragma once
#include "platform.h"
#include "Object3D.h"

class CGeometry;
class CMaterial;

class CMesh : public CObject3D
{
public:
	TYPE("Mesh");
	CMesh();
	CMesh(CGeometry *geometry, CMaterial *material = NULL);
	CMesh(string filename);

public:
	float			boundRadius;
};