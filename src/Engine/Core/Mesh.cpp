#include "platform.h"
#include "Mesh.h"
#include "Geometry.h"
#include "Material.h"
#include "Utils.h"
#include "Shader.h"
#include "ModelLoader.h"

CMesh::CMesh()
{
	geometry = NULL;
	material = NULL;
	boundRadius = 0;
}

CMesh::CMesh( CGeometry *geometry, CMaterial *material ) : CObject3D()
{
	this->geometry = geometry;
	this->material = material;

	geometry->ComputeBoundingShape();
	boundRadius = geometry->boundingSphere;
}







