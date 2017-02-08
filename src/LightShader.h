#pragma once

#include "platform.h"
#include <d3d10.h>
#include <D3DX10math.h>
#include <fstream>

using namespace std;

class CLightShader
{
public:
	CLightShader(void);
	~CLightShader(void);

	bool	Init(HWND hwnd);
	void	Release();
	void	Render(int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D10ShaderResourceView*, D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4 ambient, D3DXVECTOR4 specular, float specPower, D3DXVECTOR3 cameraPosition);

private:
	bool	InitializeShader(HWND, char*);
	void	ReleaseShader();
	void	OutputShaderErrorMessage(ID3D10Blob*, HWND, char*);

	void	SetShaderParameters(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D10ShaderResourceView*, D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4 ambient, D3DXVECTOR4 specular, float specPower, D3DXVECTOR3 cameraPosition);
	void	RenderShader(int);

private:
	ID3D10Effect				*pEffect;
	ID3D10EffectTechnique		*pTechnique;
	ID3D10InputLayout			*pLayout;

	ID3D10EffectMatrixVariable	*pWorldMatrix;
	ID3D10EffectMatrixVariable	*pViewMatrix;
	ID3D10EffectMatrixVariable	*pProjectionMatrix;

	ID3D10EffectShaderResourceVariable	*pTexture;
	ID3D10EffectVectorVariable			*pLightDirection;
	ID3D10EffectVectorVariable			*pDiffuseColor;
	ID3D10EffectVectorVariable			*pAmbientColor;
	ID3D10EffectVectorVariable			*pSpecularColor;
	ID3D10EffectScalarVariable			*pSpecularPower;
	ID3D10EffectVectorVariable			*pCameraPosition;
};
