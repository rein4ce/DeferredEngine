#pragma once
#include "platform.h"
#include "Object3D.h"
#include "Frustum.h"
#include "Matrix.h"
#include "Vector.h"

class CCamera : public CObject3D
{
public:
	CCamera(void);
	~CCamera(void);

	TYPE("Camera");

	void	CreateOrthographicView();
	void	SetView();
	
	
	//void	Render();
	virtual void		UpdateProjectionMatrix() { CreateOrthographicView(); };
	
	void				SetViewOffset(int width, int height);
	inline void			GetViewMatrix(D3DXMATRIX& viewMatrix) { viewMatrix = matView; };	

	inline void			CalculateFrustum() 
	{ 
		frustum.Construct(far, matrixProjectionScreen); 
	}

public:
	CFrustum			frustum;

	Matrix4				matrixProjection;


	Matrix4				matrixView;					// how the camera perceives the world - inverse of world matrix
	Matrix4				matrixProjectionScreen;

	float				aspect;
	float				near;
	float				far;
	
protected:
	D3DXMATRIX			matView;	
	D3DXMATRIX			matRotation;	
	
	int					width, height;
	
	const Vector3		dV, dU;				// default view and up vectors	
};

//////////////////////////////////////////////////////////////////////////
class CPerspectiveCamera : public CCamera
{
public:
	TYPE("Perspective Camera");
	CPerspectiveCamera(float fov = 90.0f, int width = 800, int height = 600, float near = 1.0f, float far = 1000.0f);

	virtual void		UpdateProjectionMatrix();
	

	inline void			CalculateFrustum() 
	{ 
		frustum.Construct(far, matrixProjectionScreen); 
	}

public:
	float			fov;	
};
