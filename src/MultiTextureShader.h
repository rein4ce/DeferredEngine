#pragma once

#include "platform.h"
#include <d3d10.h>
#include <D3DX10math.h>
#include <fstream>

using namespace std;

class CMultCTextureShader
{
public:
	CMultCTextureShader(void);
	~CMultCTextureShader(void);

	bool	Init(HWND hwnd);
	void	Release();
	void	Render(int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D10ShaderResourceView**);

private:
	bool	InitializeShader(HWND, char*);
	void	ReleaseShader();
	void	OutputShaderErrorMessage(ID3D10Blob*, HWND, char*);

	void	SetShaderParameters(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D10ShaderResourceView**);
	void	RenderShader(int);

private:
	ID3D10Effect				*pEffect;
	ID3D10EffectTechnique		*pTechnique;
	ID3D10InputLayout			*pLayout;

	ID3D10EffectMatrixVariable	*pWorldMatrix;
	ID3D10EffectMatrixVariable	*pViewMatrix;
	ID3D10EffectMatrixVariable	*pProjectionMatrix;

	ID3D10EffectShaderResourceVariable	*pTextureArray;
};
