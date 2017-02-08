#pragma once
#include "platform.h"
#include "Object3D.h"
#include "Vector.h"
#include "Matrix.h"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////////
// Light flags
//////////////////////////////////////////////////////////////////////////
#define LF_LIGHT_SPOT				(1<<0)
#define LF_LIGHT_DIRECTIONAL		(1<<1)
#define LF_LIGHT_POINT				(1<<2)
#define LF_CAST_SHADOW				(1<<3)
#define LF_OVERBRIGHT				(1<<4)				// the light will glow (override material color)


//////////////////////////////////////////////////////////////////////////
// Shader light structure
// Contains data about all lights in the scene
//////////////////////////////////////////////////////////////////////////
#pragma pack 4
struct SShaderLight 
{
	Vector4		color;
	Vector4		specular;		// 96	
	Vector3		position;		// 16
	float		range;
	Vector3		direction;		// 32	
	float		falloff;
	int			flags;	
	float		innerAngle;
	float		outerAngle;	
	float		padding;
	
	
	SShaderLight() { ZeroMemory(this, 0, sizeof(SShaderLight)); }
};



//////////////////////////////////////////////////////////////////////////
// Base Light class - abstract
//////////////////////////////////////////////////////////////////////////
class CLight : public CObject3D
{
	TYPE("Light");

public:
	Matrix4 GetViewMatrix();

	virtual SShaderLight &GetShaderLight() = 0;
	virtual Matrix4 GetProjectionMatrix() = 0;	

protected:
	CLight();	

public:	
	SRGBA		color;
	SRGBA		specular;	
	bool		overbright;
	float		intensity;

	Matrix4		viewMatrix;
	Matrix4		projectionMatrix;

	float		shadowNear;
	float		shadowFar;

	// directional
	int			width, height;

	// point + spot
	float		range;	

	// spot light
	float		innerAngle;
	float		outerAngle;
	float		falloff;
};

//////////////////////////////////////////////////////////////////////////
class CDirectionalLight : public CLight
{
	TYPE("DirectionalLight");
public:
	CDirectionalLight(Vector3 direction, SRGBA &color = WHITE, SRGBA &specular = WHITE);

	virtual SShaderLight &GetShaderLight();	
	virtual Matrix4 GetProjectionMatrix();	
};

//////////////////////////////////////////////////////////////////////////
class CPointLight : public CLight
{
	TYPE("PointLight");
public:
	CPointLight(Vector3 position, SRGBA &color = WHITE, float range = 5.0f, SRGBA &specular = WHITE);

	virtual SShaderLight &GetShaderLight();
	virtual Matrix4 GetProjectionMatrix();	
};

//////////////////////////////////////////////////////////////////////////
class CSpotLight : public CLight
{
	TYPE("SpotLight");
public:
	CSpotLight(Vector3 position, Vector3 direction, SRGBA color, float range = 5.0f, float inner = 1.0f, float outer = 3.0f, float falloff = 3.0f);

	virtual SShaderLight &GetShaderLight();
	virtual Matrix4 GetProjectionMatrix();
	
};
