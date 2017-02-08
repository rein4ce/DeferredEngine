#include "platform.h"
#include "TextureArray.h"

CTextureArray::CTextureArray()
{
	pTextureArray[0] = NULL;
	pTextureArray[1] = NULL;
}

CTextureArray::~CTextureArray()
{

}

//////////////////////////////////////////////////////////////////////////
bool CTextureArray::Init(char *filename1, char *filename2)
{
	HRESULT result;
	result = D3DX10CreateShaderResourceViewFromFile(g_pDevice, filename1, NULL, NULL, &pTextureArray[0], NULL);
	if (FAILED(result)) 
	{
		trace("Error while loading texture from file %s: %d", filename1, result);
		return false;
	}

	result = D3DX10CreateShaderResourceViewFromFile(g_pDevice, filename2, NULL, NULL, &pTextureArray[1], NULL);
	if (FAILED(result)) 
	{
		trace("Error while loading texture from file %s: %d", filename2, result);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTextureArray::Release()
{
	SAFE_RELEASE(pTextureArray[0]);
	SAFE_RELEASE(pTextureArray[1]);
}