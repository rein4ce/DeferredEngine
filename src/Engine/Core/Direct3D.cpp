#include "platform.h"
#include "Direct3D.h"
#include "Utils.h"
#include "Utils3D.h"

Direct3D::Direct3D(void)
{
	pSwapChain = NULL;
	pDevice = NULL;
	pRenderTargetView = NULL;
	pDepthStencilView = NULL;		
}

Direct3D::~Direct3D(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Main init function
//////////////////////////////////////////////////////////////////////////
bool Direct3D::Init(int width, int height, bool vsync, HWND hwnd, bool fullscreen, float depth, float _near)
{
	HRESULT result;
	
	IDXGIFactory *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput *adapterOutput;

	this->vsync = vsync;

	// Create directX graphics interface factory
	if ( FAILED( CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory) )) return false;

	// Create an adapter for the primary graphics interface
	if ( FAILED( factory->EnumAdapters(0, &adapter))) return false;

	// Enumerate te pimary adapter
	if ( FAILED( adapter->EnumOutputs( 0, &adapterOutput ))) return false;

	// Get supported modes
	uint32 numModes;
	if ( FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL ))) return false;

	// Get a list of all supported video modes
	DXGI_MODE_DESC *displayModeList = new DXGI_MODE_DESC[numModes];
	if ( !displayModeList ) return false;

	// Find the mode that matches the given criteria
	uint32 numerator = 0, denominator = 1;
	for (int i = 0; i < numModes; i++ )
	{
		if ( displayModeList[i].Width == width && displayModeList[i].Height == height ) 
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	// Get video card parameters
	DXGI_ADAPTER_DESC adapterDesc;
	if ( FAILED( adapter->GetDesc( &adapterDesc ))) return false;

	// Clean up the interface
	SAFE_DELETE_ARRAY( displayModeList );
	SAFE_RELEASE( adapterOutput );
	SAFE_RELEASE( adapter );
	SAFE_RELEASE( factory );

	
	// Create swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof(swapChainDesc) );
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if ( vsync )
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else 
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	}
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !fullscreen;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	
	// Create the device
	result = D3D10CreateDeviceAndSwapChain( 
		NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&swapChainDesc,
		&pSwapChain,
		&pDevice );
	if ( FAILED( result )) return false;

	// Attach the back bufer to the swap chain
	ID3D10Texture2D *pBackBuffer;
	if ( FAILED( pSwapChain->GetBuffer( 0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer ))) return false;

	// Create render target view
	if ( FAILED( pDevice->CreateRenderTargetView( pBackBuffer, NULL, &pRenderTargetView ))) return false;

	SAFE_RELEASE(pBackBuffer);

	// Create depth buffer
	D3D10_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory( &depthBufferDesc, sizeof(depthBufferDesc) );
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D10_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	if ( FAILED( pDevice->CreateTexture2D( &depthBufferDesc, NULL, &pDepthStencilBuffer ))) return false;

	// Setup deph stencil buffer parameters
	D3D10_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.	
	depthStencilDesc.StencilEnable = true;	
	depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;	
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	// Create possible render states configuration
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	if ( FAILED( pDevice->CreateDepthStencilState( &depthStencilDesc, &pDepthStencilStates[OM_DEPTH_TEST | OM_DEPTH_WRITE] ))) return false;

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
	if ( FAILED( pDevice->CreateDepthStencilState( &depthStencilDesc, &pDepthStencilStates[0] ))) return false;

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
	if ( FAILED( pDevice->CreateDepthStencilState( &depthStencilDesc, &pDepthStencilStates[OM_DEPTH_TEST] ))) return false;

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	if ( FAILED( pDevice->CreateDepthStencilState( &depthStencilDesc, &pDepthStencilStates[OM_DEPTH_WRITE] ))) return false;
	
	// Set the buffer
	pDevice->OMSetDepthStencilState( pDepthStencilStates[OM_DEPTH_TEST | OM_DEPTH_WRITE], 1 );

	// Depth buffer view
	D3D10_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if ( FAILED( pDevice->CreateDepthStencilView(pDepthStencilBuffer, &depthStencilViewDesc, &pDepthStencilView ))) return false;

	
	// Bind the target view and the buffers
	SetDefaultRenderTarget();

	// Setup rasterizer
	D3D10_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D10_CULL_NONE;//*/D3D10_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.FillMode = D3D10_FILL_SOLID;//D3D10_FILL_WIREFRAME;//
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	if ( FAILED( pDevice->CreateRasterizerState( &rasterDesc, &pRasterStates[0] ))) return false;

	rasterDesc.FillMode = D3D10_FILL_WIREFRAME;
	if ( FAILED( pDevice->CreateRasterizerState( &rasterDesc, &pRasterStates[RS_WIREFRAME] ))) return false;

	rasterDesc.CullMode = D3D10_CULL_BACK;
	if ( FAILED( pDevice->CreateRasterizerState( &rasterDesc, &pRasterStates[RS_CULL | RS_WIREFRAME] ))) return false;

	rasterDesc.FillMode = D3D10_FILL_SOLID;
	if ( FAILED( pDevice->CreateRasterizerState( &rasterDesc, &pRasterStates[RS_CULL] ))) return false;

	pDevice->RSSetState(pRasterStates[RS_CULL]);

	// Create viewport	
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	pDevice->RSSetViewports(1, &viewport);

	// Init blend states
	InitBlendStates();

	return true;
}


//////////////////////////////////////////////////////////////////////////
void Direct3D::Shutdown() 
{
	if ( pSwapChain ) pSwapChain->SetFullscreenState(false, NULL);
	for (int i=0; i<4; i++) SAFE_RELEASE(pRasterStates[i]);
	SAFE_RELEASE(pDepthStencilView);
	for (int i=0; i<4; i++) SAFE_RELEASE(pDepthStencilStates[i]);
	SAFE_RELEASE(pDepthStencilBuffer);	
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pSwapChain);
}

//////////////////////////////////////////////////////////////////////////
// Clear the back buffer and stencil buffer
//////////////////////////////////////////////////////////////////////////
void Direct3D::Clear(bool clearColor, bool clearDepthStencil, SRGBA color)
{
	float c[4] = { (float)color.r/255.0f,(float) color.g/255.0f, (float)color.g/255.0f, 255 };
	if (clearColor) pDevice->ClearRenderTargetView(pRenderTargetView, c);
	if (clearDepthStencil) pDevice->ClearDepthStencilView(pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::Display()
{
	pSwapChain->Present( vsync ? 1 : 0, 0);
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::InitBlendStates()
{
	D3D10_BLEND_DESC blend;
	ZeroMemory(&blend, sizeof(D3D10_BLEND_DESC));

	// Normal
	blend.BlendEnable[0] = true;
	blend.BlendOp = D3D10_BLEND_OP_ADD;
	blend.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	blend.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;	
	blend.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D10_BLEND_ONE;	
	blend.DestBlendAlpha = D3D10_BLEND_ONE;	
	blend.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
	//blend.AlphaToCoverageEnable = true;

	DXV(pDevice->CreateBlendState(&blend, &pBlendNormal));

	// Additive
	blend.BlendEnable[0] = true;
	blend.BlendOp = D3D10_BLEND_OP_ADD;
	blend.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	blend.DestBlend = D3D10_BLEND_DEST_ALPHA;	
	blend.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D10_BLEND_ONE;	
	blend.DestBlendAlpha = D3D10_BLEND_ONE;	
	blend.AlphaToCoverageEnable = false;

	DXV(pDevice->CreateBlendState(&blend, &pBlendAdditive));

	// Additive Lights (G-Buffer)
	blend.BlendEnable[0] = true;
	blend.BlendOp = D3D10_BLEND_OP_ADD;
	blend.SrcBlend = D3D10_BLEND_ONE;
	blend.DestBlend = D3D10_BLEND_ONE;	
	blend.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D10_BLEND_ONE;	
	blend.DestBlendAlpha = D3D10_BLEND_ONE;	
	blend.AlphaToCoverageEnable = false;

	DXV(pDevice->CreateBlendState(&blend, &pBlendAdditiveLights));

	// Subtractive
	blend.BlendOp = D3D10_BLEND_OP_SUBTRACT;
	blend.DestBlend = D3D10_BLEND_ONE;
	blend.SrcBlend = D3D10_BLEND_ONE;
	blend.BlendOpAlpha = D3D10_BLEND_OP_SUBTRACT;
	blend.DestBlendAlpha = D3D10_BLEND_ONE;
	blend.SrcBlendAlpha = D3D10_BLEND_ONE;	

	DXV(pDevice->CreateBlendState(&blend, &pBlendSubtractive));

	// None
	for (int i=0; i<MAX_GBUFFERS_NUM; i++)
	{
		blend.BlendEnable[i] = false;
		blend.RenderTargetWriteMask[i] = D3D10_COLOR_WRITE_ENABLE_ALL;
	}	
	blend.BlendOp = D3D10_BLEND_OP_ADD;
	blend.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	blend.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;	
	blend.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D10_BLEND_ONE;	
	blend.DestBlendAlpha = D3D10_BLEND_ONE;	

	DXV(pDevice->CreateBlendState(&blend, &pBlendNone));
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::SetBlending( EBlending::Type blending )
{
	switch (blending)
	{
	case EBlending::Additive:
		pDevice->OMSetBlendState(pBlendAdditive, 0, 0xffffffff);
		break;

	case EBlending::AdditiveLights:
		pDevice->OMSetBlendState(pBlendAdditiveLights, 0, 0xffffffff);
		break;

	case EBlending::Subtractive:
		pDevice->OMSetBlendState(pBlendSubtractive, 0, 0xffffffff);
		break;

	case EBlending::None:
		pDevice->OMSetBlendState(pBlendNone, 0, 0xffffffff);
		break;

	default:
		pDevice->OMSetBlendState(pBlendNormal, 0, 0xffffffff);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::SetStencilState(bool test, bool write)
{
	int state = 0;
	if (test) state |= OM_DEPTH_TEST;
	if (write) state |= OM_DEPTH_WRITE;
	pDevice->OMSetDepthStencilState( pDepthStencilStates[state], 1 );
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::SetDefaultRenderTarget()
{
	pDevice->OMSetRenderTargets( 1, &pRenderTargetView, pDepthStencilView );
	pDevice->RSSetViewports(1, &viewport);
}

//////////////////////////////////////////////////////////////////////////
void Direct3D::SetRasterState(int state)
{
	pDevice->RSSetState(pRasterStates[state]);
}