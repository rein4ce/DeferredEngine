#pragma once

#include "platform.h"
#include <D3DX10math.h>
#include "Shader.h"
#include "Vector.h"

using namespace std;

class CMaterial;

//////////////////////////////////////////////////////////////////////////
class CColorShader : public CShaderOld 
{
public:
	CColorShader(void) {};

	virtual bool Init(ID3D10Device *pDevice) 
	{
		if (!LoadShader("ColorShader", "shaders/other/color.fx", pDevice)) return false;

		// Layout
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT	,	0, 12,	D3D10_INPUT_PER_VERTEX_DATA, 0 }			
		};
		SetLayout(layout, 2);

		// Variables
		pWorldMatrix		= pEffect->GetVariableByName("worldMatrix")->AsMatrix();
		pViewMatrix			= pEffect->GetVariableByName("viewMatrix")->AsMatrix();
		pProjectionMatrix	= pEffect->GetVariableByName("projectionMatrix")->AsMatrix();
		return true;
	}

public:
	ID3D10EffectMatrixVariable			*pWorldMatrix;
	ID3D10EffectMatrixVariable			*pViewMatrix;
	ID3D10EffectMatrixVariable			*pProjectionMatrix;
};

