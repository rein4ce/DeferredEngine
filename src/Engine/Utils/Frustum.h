#pragma once
#include "platform.h"

struct Matrix4;
class CObject3D;

class CFrustum
{
public:
	CFrustum();
	~CFrustum();

	void	Construct(float, Matrix4 &matrixProjectionScreen);

	bool	CheckPoint(float x, float y, float z);
	bool	CheckCube(float x, float y, float z, float size);
	bool	CheckSphere(float, float, float, float);
	bool	CheckBox(float, float, float, float, float, float);
	bool	CheckMesh(CObject3D &mesh);

private:
	D3DXPLANE	planes[6];
};