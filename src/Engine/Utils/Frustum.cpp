#include "platform.h"
#include "Frustum.h"
#include "Mesh.h"
#include "Geometry.h"
#include "Matrix.h"

CFrustum::CFrustum() {}
CFrustum::~CFrustum() {}

//////////////////////////////////////////////////////////////////////////
void CFrustum::Construct( float depth, Matrix4 &matrixProjectionScreen )
{
	Matrix4 matrix;
	matrix = matrixProjectionScreen; 

	planes[0].a = matrix.n41 - matrix.n11;
	planes[0].b = matrix.n42 - matrix.n12;
	planes[0].c = matrix.n43 - matrix.n13;
	planes[0].d = matrix.n44 - matrix.n14;
	D3DXPlaneNormalize(&planes[0], &planes[0]);

	planes[1].a = matrix.n41 + matrix.n11;
	planes[1].b = matrix.n42 + matrix.n12;
	planes[1].c = matrix.n43 + matrix.n13;
	planes[1].d = matrix.n44 + matrix.n14;
	D3DXPlaneNormalize(&planes[1], &planes[1]);

	planes[2].a = matrix.n41 + matrix.n21;
	planes[2].b = matrix.n42 + matrix.n22;
	planes[2].c = matrix.n43 + matrix.n23;
	planes[2].d = matrix.n44 + matrix.n24;
	D3DXPlaneNormalize(&planes[2], &planes[2]);

	planes[3].a = matrix.n41 - matrix.n21;
	planes[3].b = matrix.n42 - matrix.n22;
	planes[3].c = matrix.n43 - matrix.n23;
	planes[3].d = matrix.n44 - matrix.n24;
	D3DXPlaneNormalize(&planes[3], &planes[3]);

	planes[4].a = matrix.n41 + matrix.n31;
	planes[4].b = matrix.n42 + matrix.n32;
	planes[4].c = matrix.n43 + matrix.n33;
	planes[4].d = matrix.n44 + matrix.n34;
	D3DXPlaneNormalize(&planes[4], &planes[4]);

	planes[5].a = matrix.n41 - matrix.n31;
	planes[5].b = matrix.n42 - matrix.n32;
	planes[5].c = matrix.n43 - matrix.n33;
	planes[5].d = matrix.n44 - matrix.n34;
	D3DXPlaneNormalize(&planes[5], &planes[5]);
}

//////////////////////////////////////////////////////////////////////////
bool CFrustum::CheckPoint(float x, float y, float z)
{
	for (int i = 0; i < 6; i++)
		if ( D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x,y,z)) < 0.0f ) 
			return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CFrustum::CheckCube(float x, float y, float z, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-radius, y-radius, z-radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+radius, y-radius, z-radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-radius, y+radius, z-radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+radius, y+radius, z-radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-radius, y-radius, z+radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+radius, y-radius, z+radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-radius, y+radius, z+radius)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+radius, y+radius, z+radius)) >= 0.0f) continue;
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CFrustum::CheckSphere(float x, float y, float z, float radius)
{
	for (int i = 0; i < 6; i++)
		if ( D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x,y,z)) < -radius ) 
			return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CFrustum::CheckBox(float x, float y, float z, float sizeX, float sizeY, float sizeZ)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-sizeX, y-sizeY, z-sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+sizeX, y-sizeY, z-sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-sizeX, y+sizeY, z-sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+sizeX, y+sizeY, z-sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-sizeX, y-sizeY, z+sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+sizeX, y-sizeY, z+sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x-sizeX, y+sizeY, z+sizeZ)) >= 0.0f) continue;
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(x+sizeX, y+sizeY, z+sizeZ)) >= 0.0f) continue;
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Check if given object is in frustum
//////////////////////////////////////////////////////////////////////////
bool CFrustum::CheckMesh( CObject3D &mesh )
{
	Vector3 pos = mesh.GetWorldPosition();

	if (mesh.boundingShape == EBoundingShape::Box)
	{
		Vector3 size = mesh.geometry->boundingBox.GetSize();
		size.x *= mesh.scale.x;
		size.y *= mesh.scale.y;
		size.z *= mesh.scale.z;
		Vector3 pos = mesh.GetWorldPosition();
		pos.x += (mesh.geometry->boundingBox.min.x + size.x/2) * mesh.scale.x;
		pos.y += (mesh.geometry->boundingBox.min.y + size.y/2) * mesh.scale.y;
		pos.z += (mesh.geometry->boundingBox.min.z + size.z/2) * mesh.scale.z;

		return CheckBox(pos.x, pos.y, pos.z, size.x, size.y, size.z);
	}
	else
	{
		float maxScale = max(max(mesh.scale.x, mesh.scale.y), mesh.scale.z);
		float boundingRadius = maxScale * mesh.geometry->boundingSphere;	
		return CheckSphere(pos.x, pos.y, pos.z, boundingRadius);
	}
}
