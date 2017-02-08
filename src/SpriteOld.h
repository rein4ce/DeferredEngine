#pragma once
#include "platform.h"
#include "VoidGame.h"
#include <d3d10.h>
#include <D3DX10math.h>
#include "Resources.h"

class CSpriteOld
{
private:

	struct SVertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

public:
	CSpriteOld(void);
	~CSpriteOld(void);

	bool	Init(char *filename, int width, int height);
	void	Release();
	void	Render(int x, int y);

	inline int		GetIndexCount() { return indexCount; };
	inline ID3D10ShaderResourceView *GetTexture() { return pTexture->GetTexture(); };


private:
	bool	InitializeBuffers();
	void	ShutdownBuffers();
	void	RenderBuffers();
	bool	LoadTexture(char* filename);
	void	ReleaseTexture();
	bool	UpdateBuffers(int x, int y);

private:
	ID3D10Buffer	*pVertexBuffer, *pIndexBuffer;
	int				vertexCount, indexCount;
	CTexture		*pTexture;

public:
	int				width, height;
	int				lastX, lastY;
};
