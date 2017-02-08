#include "LightShader.h"
#include <sstream>

extern ID3D10Device*				g_pDevice;

CLightShader::CLightShader(void)
{
	pEffect = NULL;
	pTechnique = NULL;
	pLayout = NULL;
	pTexture = NULL;
	pLightDirection = NULL;
	pDiffuseColor = NULL;
	pAmbientColor = NULL;
	pSpecularPower = NULL;
	pSpecularColor = NULL;
	pCameraPosition = NULL;

	pWorldMatrix = NULL;
	pViewMatrix = NULL;
	pProjectionMatrix = NULL;
}

CLightShader::~CLightShader(void)
{
}

//////////////////////////////////////////////////////////////////////////
bool CLightShader::Init(HWND hwnd)
{
	if ( !InitializeShader(hwnd, "shaders/light.fx" )) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::Release()
{
	ReleaseShader();
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::Render(int indexCount, D3DXMATRIX matWorld, D3DXMATRIX matView, D3DXMATRIX matProjection, ID3D10ShaderResourceView *texture, D3DXVECTOR3 direction, D3DXVECTOR4 diffuse, D3DXVECTOR4 ambient, D3DXVECTOR4 specular, float specPower, D3DXVECTOR3 cameraPosition)
{
	SetShaderParameters(matWorld, matView, matProjection, texture, direction, diffuse, ambient, specular, specPower, cameraPosition);
	RenderShader(indexCount);
}

//////////////////////////////////////////////////////////////////////////
bool CLightShader::InitializeShader(HWND hwnd, char* filename)
{
	HRESULT result;
	ID3D10Blob *error;

	if ( FAILED(result = D3DX10CreateEffectFromFile(filename, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, g_pDevice, NULL, NULL, &pEffect, &error, NULL ))) 
	{
		if (error)
		{
			OutputShaderErrorMessage(error, hwnd, filename);
		}
		else
		{
			char msg[128];
			sprintf(msg,"Unable to load shader file %s", filename);
			MessageBox(hwnd, filename, msg, MB_OK);
		}
		return false;
	}

	pTechnique = pEffect->GetTechniqueByName("LightTechnique");
	if (!pTechnique) return false;

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D10_INPUT_PER_VERTEX_DATA, 0 }			
	};

	int numElements = sizeof(layout) / sizeof(layout[0]);

	D3D10_PASS_DESC passDesc;
	pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	if ( FAILED(result = g_pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pLayout ))) 
	{
		trace("Error intializing texture shader CreateInputLayout");
		return false;
	}

	pWorldMatrix = pEffect->GetVariableByName("worldMatrix")->AsMatrix();
	pViewMatrix = pEffect->GetVariableByName("viewMatrix")->AsMatrix();
	pProjectionMatrix = pEffect->GetVariableByName("projectionMatrix")->AsMatrix();
	pTexture = pEffect->GetVariableByName("shaderTexture")->AsShaderResource();
	pLightDirection = pEffect->GetVariableByName("lightDirection")->AsVector();
	pDiffuseColor = pEffect->GetVariableByName("diffuseColor")->AsVector();
	pAmbientColor = pEffect->GetVariableByName("ambientColor")->AsVector();
	pSpecularColor = pEffect->GetVariableByName("specularColor")->AsVector();
	pSpecularPower = pEffect->GetVariableByName("specularPower")->AsScalar();
	pCameraPosition = pEffect->GetVariableByName("cameraPosition")->AsVector();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::ReleaseShader()
{
	pWorldMatrix = NULL;
	pViewMatrix = NULL;
	pProjectionMatrix = NULL;
	pTechnique = NULL;
	pTexture = NULL;
	pLightDirection = NULL;
	pDiffuseColor = NULL;
	pAmbientColor = NULL;
	pSpecularPower = NULL;
	pSpecularColor = NULL;
	pCameraPosition = NULL;

	SAFE_RELEASE(pLayout);
	SAFE_RELEASE(pEffect);
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::OutputShaderErrorMessage(ID3D10Blob *error, HWND hwnd, char *filename)
{
	char *compileErrors;
	uint32 bufferSize;

	compileErrors = (char*)error->GetBufferPointer();
	bufferSize = error->GetBufferSize();

	string fout;
	fout = string("Error compiling shader ") + filename + "\n\n";
	for (int i=0; i<bufferSize; i++)
		fout += compileErrors[i];


	SAFE_RELEASE(error);
	MessageBox(hwnd, fout.c_str(), "Error", MB_OK);
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::SetShaderParameters(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D10ShaderResourceView *texture, D3DXVECTOR3 direction, D3DXVECTOR4 diffuse, D3DXVECTOR4 ambient, D3DXVECTOR4 specular, float specPower, D3DXVECTOR3 cameraPosition)
{
	pWorldMatrix->SetMatrix((float*)&worldMatrix);
	pViewMatrix->SetMatrix((float*)&viewMatrix);
	pProjectionMatrix->SetMatrix((float*)&projectionMatrix);
	pTexture->SetResource(texture);
	pDiffuseColor->SetFloatVector((float*)&diffuse);
	pLightDirection->SetFloatVector((float*)&direction);
	pAmbientColor->SetFloatVector((float*)&ambient);
	pSpecularColor->SetFloatVector((float*)&specular);
	pSpecularPower->SetFloat(specPower);
	pCameraPosition->SetFloatVector((float*)&cameraPosition);
}

//////////////////////////////////////////////////////////////////////////
void CLightShader::RenderShader(int indexCount)
{
	D3D10_TECHNIQUE_DESC techniqueDesc;

	g_pDevice->IASetInputLayout(pLayout);
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		pTechnique->GetPassByIndex(i)->Apply(0);
		g_pDevice->DrawIndexed(indexCount, 0, 0);
	}
}