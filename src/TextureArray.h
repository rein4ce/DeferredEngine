#pragma once

#include "platform.h"
#include <D3D10.h>
#include <D3DX10.h>
#include "VoidGame.h"

class CTextureArray
{
public:
	CTextureArray();
	~CTextureArray();

	bool	Init(char *filename1, char *filename2);
	void	Release();

	inline ID3D10ShaderResourceView **GetTextureArray() { return pTextureArray; };

private:
	ID3D10ShaderResourceView *pTextureArray[2];
};