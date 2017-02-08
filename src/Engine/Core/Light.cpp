#include "platform.h"
#include "Light.h"
#include "Shader.h"
#include "CVar.h"
#include "Utils.h"
#include "Vector.h"

extern CVar cv_shadowNear;
extern CVar cv_shadowFar;



Matrix4 CLight::GetViewMatrix() 
{ 
	viewMatrix.GetInverse(matrixWorld);
	return viewMatrix;
}

//////////////////////////////////////////////////////////////////////////
CLight::CLight() 
{
	color = WHITE;
	specular = WHITE;
	castShadow = false;
	overbright = false;
	shadowNear = 1;
	shadowFar = 100;
}

//////////////////////////////////////////////////////////////////////////
CDirectionalLight::CDirectionalLight(Vector3 direction, SRGBA &color, SRGBA &specular) : CLight()
{
	this->SetDirection(direction);
	this->color = color;
	this->intensity = color.a / 255.0f;
	this->specular = color;
	width = 400;
	height = 300;
}	

SShaderLight& CDirectionalLight::GetShaderLight()
{
	SShaderLight s;
	s.flags			= LF_LIGHT_DIRECTIONAL;
	if (castShadow)	s.flags |= LF_CAST_SHADOW;
	if (overbright) s.flags |= LF_OVERBRIGHT;

	s.direction		= forward;
	s.color			= color;
	s.color.w		= intensity;
	s.specular		= specular;
	return s;
}


Matrix4 CDirectionalLight::GetProjectionMatrix()
{
	D3DXMATRIX m;
	D3DXMatrixOrthoOffCenterLH(&m, -width / 2, +width / 2, -height / 2, +height / 2, shadowNear, shadowFar);
	return projectionMatrix = Matrix4(m);
}


//////////////////////////////////////////////////////////////////////////
CPointLight::CPointLight(Vector3 position, SRGBA &color, float range, SRGBA &specular) : CLight()
{
	this->position = position;
	this->color = color;
	this->intensity = color.a / 255.0f;
	this->specular = color;
	this->range = range;
}

Matrix4 CPointLight::GetProjectionMatrix()
{
	return Matrix4();
}

SShaderLight& CPointLight::GetShaderLight()
{
	SShaderLight s;
	s.flags			= LF_LIGHT_POINT;
	if (castShadow)	s.flags |= LF_CAST_SHADOW;
	if (overbright) s.flags |= LF_OVERBRIGHT;

	s.position		= position;
	s.color			= color;
	s.color.w		= intensity;
	s.specular		= specular;
	s.range			= range;
	return s;
}


//////////////////////////////////////////////////////////////////////////
CSpotLight::CSpotLight( Vector3 position, Vector3 direction, SRGBA color, float range /*= 5.0f*/, float inner /*= 1.0f*/, float outer /*= 3.0f*/, float falloff /*= 3.0f*/ )
{
	this->position = position;
	this->SetDirection(direction);
	this->color = color;
	this->specular = color;
	this->intensity = color.a / 255.0f;
	this->range = range;
	this->innerAngle = inner;
	this->outerAngle = outer;
	this->falloff = falloff;
}

Matrix4 CSpotLight::GetProjectionMatrix()
{
	return projectionMatrix = Matrix4::MakePerspective(90.0f, 1, shadowNear, shadowFar);	
}

SShaderLight& CSpotLight::GetShaderLight()
{
	SShaderLight s;
	s.flags			= LF_LIGHT_SPOT;
	if (castShadow)	s.flags |= LF_CAST_SHADOW;
	if (overbright) s.flags |= LF_OVERBRIGHT;

	s.position		= position;
	s.direction		= forward;
	s.color			= color;
	s.color.w		= intensity;
	s.specular		= specular;
	s.range			= range;
	s.innerAngle	= innerAngle;
	s.outerAngle	= outerAngle;
	s.falloff		= falloff;
	return s;
}
