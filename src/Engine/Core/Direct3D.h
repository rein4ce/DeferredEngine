#pragma once
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")

#include "platform.h"
#include "Utils3D.h"

// G-Buffer:
//	color
//	normals
//	depth
//	light
#define MAX_GBUFFERS_NUM		4

struct SRGBA;

// Render states
#define OM_DEPTH_TEST	(1<<0)
#define OM_DEPTH_WRITE	(1<<1)

#define RS_CULL			(1<<0)
#define RS_WIREFRAME	(1<<1)

class Direct3D
{
public:
	Singleton(Direct3D);
	~Direct3D(void);

	bool	Init(int width, int height, bool vsync, HWND hwnd, bool fullscreen, float depth, float near);
	void	Shutdown();
	void	Clear(bool clearColor, bool clearDepthStencil, SRGBA color);
	void	Display();

	void	SetRasterState(int state);
	void	SetBlending(EBlending::Type blending);
	void	SetStencilState(bool test, bool write);
	void	SetDefaultRenderTarget();

public:
	inline ID3D10Device*			GetDevice() { return pDevice; };

	inline void						SetWireframeState() { SetRasterState(RS_WIREFRAME); }
	inline void						SetFillState() { SetRasterState(RS_CULL); }

	static inline ID3D10Device*		Device() { return Direct3D::Instance()->GetDevice(); }

protected:
	void							InitBlendStates();

public:
	IDXGISwapChain*					pSwapChain;
	ID3D10Device*					pDevice;
	ID3D10RenderTargetView*			pRenderTargetView;
	ID3D10DepthStencilState*		pDepthStencilStates[4];	
	ID3D10DepthStencilView*			pDepthStencilView;
	ID3D10Texture2D*				pDepthStencilBuffer;
	ID3D10RasterizerState*			pRasterStates[4];

	ID3D10BlendState*				pBlendAdditive;
	ID3D10BlendState*				pBlendSubtractive;
	ID3D10BlendState*				pBlendNormal;
	ID3D10BlendState*				pBlendNone;	
	ID3D10BlendState*				pBlendAdditiveLights;	

	D3D10_VIEWPORT					viewport;

	// Settings
	bool							vsync;
};
