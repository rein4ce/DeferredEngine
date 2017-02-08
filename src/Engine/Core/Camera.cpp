#include "platform.h"
#include "Camera.h"
#include "Utils.h"

CCamera::CCamera(void) :
	CObject3D(),
	dV(Vector3(0,0,1)),
	dU(Vector3(0,1,0))
{
	position = Vector3(0,0,0);
	rotation = Vector3(0,0,0);
	forward = Vector3(0,0,1);
	up = Vector3(0,1,0);
	right = Vector3(1,0,0);

	near = 0.1f;
	far = 1000.0f;
	
	CreateOrthographicView();

	width = 0;
	height = 0;
}

CCamera::~CCamera(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CCamera::SetView()
{
	float pitch = rotation.x * ToRad;
	float yaw   = rotation.y * ToRad;
	float roll  = rotation.z * ToRad;

	D3DXMatrixRotationYawPitchRoll(&matRotation, yaw, pitch, roll);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&forward, (D3DXVECTOR3*)&dV, &matRotation);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&up, (D3DXVECTOR3*)&dU, &matRotation);

	D3DXVec3Normalize((D3DXVECTOR3*)&forward, (D3DXVECTOR3*)&forward);
	D3DXVec3Cross((D3DXVECTOR3*)&right, (D3DXVECTOR3*)&up, (D3DXVECTOR3*)&forward);
	D3DXVec3Normalize((D3DXVECTOR3*)&right, (D3DXVECTOR3*)&right);

	Vector3 at = position + forward;

	D3DXMatrixLookAtLH(&matView, (D3DXVECTOR3*)&position, (D3DXVECTOR3*)&at, (D3DXVECTOR3*)&up);
	
}

//////////////////////////////////////////////////////////////////////////
void CCamera::CreateOrthographicView()
{
	D3DXMATRIX m;
	D3DXMatrixOrthoOffCenterLH(&m, -width/2, +width/2, -height/2, +height/2, near, far);
	matrixProjection = Matrix4(m);
}

//////////////////////////////////////////////////////////////////////////
CPerspectiveCamera::CPerspectiveCamera( float fov, int width, int height, float near, float far ) : CCamera()
{
	this->fov = fov;	
	this->near = near;
	this->far = far;
	SetViewOffset(width, height);	
}

//////////////////////////////////////////////////////////////////////////
void CPerspectiveCamera::UpdateProjectionMatrix()
{
	matrixProjection = Matrix4::MakePerspective(fov, aspect, near, far);	
}

//////////////////////////////////////////////////////////////////////////
void CCamera::SetViewOffset( int width, int height )
{
	this->width = width;
	this->height = height;
	this->aspect = (float)width / height;
	UpdateProjectionMatrix();
}