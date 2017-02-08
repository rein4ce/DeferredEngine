#include "platform.h"
#include "MultiTextureShader.h"
#include "VoidGame.h"
#include <sstream>

CMultCTextureShader::CMultCTextureShader(void)
{
	pEffect = NULL;
	pTechnique = NULL;
	pLayout = NULL;
	pTextureArray = NULL;

	pWorldMatrix = NULL;
	pViewMatrix = NULL;
	pProjectionMatrix = NULL;
}

CMultCTextureShader::~CMultCTextureShader(void)
{
}

//////////////////////////////////////////////////////////////////////////
bool CMultCTextureShader::Init(HWND hwnd)
{
	if ( !InitializeShader(hwnd, "shaders/multCTexture.fx" )) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMultCTextureShader::Release()
{
	ReleaseShader();
}

//////////////////////////////////////////////////////////////////////////
void CMultCTextureShader::Render(int indexCount, D3DXMATRIX matWorld, D3DXMATRIX matView, D3DXMATRIX matProjection, ID3D10ShaderResourceView **textures)
{
	SetShaderParameters(matWorld, matView, matProjection, textures);
	RenderShader(indexCount);
}

//////////////////////////////////////////////////////////////////////////
bool CMultCTextureShader::InitializeShader(HWND hwnd, char* filename)
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

	pTechnique = pEffect->GetTechniqueByName("TextureTechnique");
	if (!pTechnique) return false;

	D3D10_INPUT_ELEMENT_DESC layout[2];
	uint32 numElements;

	layout[0].SemanticName = "POSITION";
	layout[0].SemanticIndex = 0;
	layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout[0].InputSlot = 0;
	layout[0].AlignedByteOffset = 0;
	layout[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	layout[0].InstanceDataStepRate = 0;

	layout[1].SemanticName = "TEXCOORD";
	layout[1].SemanticIndex = 0;
	layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout[1].InputSlot = 0;
	layout[1].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
	layout[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	layout[1].InstanceDataStepRate = 0;

	numElements = sizeof(layout) / sizeof(layout[0]);

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
	pTextureArray = pEffect->GetVariableByName("shaderTextures")->AsShaderResource();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMultCTextureShader::ReleaseShader()
{
	pWorldMatrix = NULL;
	pViewMatrix = NULL;
	pProjectionMatrix = NULL;
	pTechnique = NULL;
	pTextureArray = NULL;

	SAFE_RELEASE(pLayout);
	SAFE_RELEASE(pEffect);
}

//////////////////////////////////////////////////////////////////////////
void CMultCTextureShader::OutputShaderErrorMessage(ID3D10Blob *error, HWND hwnd, char *filename)
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
void CMultCTextureShader::SetShaderParameters(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D10ShaderResourceView **textures)
{
	pWorldMatrix->SetMatrix((float*)&worldMatrix);
	pViewMatrix->SetMatrix((float*)&viewMatrix);
	pProjectionMatrix->SetMatrix((float*)&projectionMatrix);
	pTextureArray->SetResourceArray(textures, 0, 2);
}

//////////////////////////////////////////////////////////////////////////
void CMultCTextureShader::RenderShader(int indexCount)
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