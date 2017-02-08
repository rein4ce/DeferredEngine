#pragma once
#include "platform.h"
#include "Direct3D.h"
#include "Shaders.h"
#include "VertexFormats.h"
#include "Camera.h"
#include "ResourceMgr.h"
#include "Utils3D.h"
#include "SlotArray.h"
#include "Array.h"

#define MAX_DEBUG_POINTS	65535
#define MAX_DEBUG_LINES		65535
#define MAX_DEBUG_TRIANGLES	65535

class CScene;
class CCamera;
class CShader;
class CMesh;
class CGeometryGroup;
struct SFog;
class CLight;
class CSprite3D;
class CSprite2D;
class CSprite;
class CSpriteMaterial;
class CGeometryRenderer;

namespace EGBuffer { enum Type 
{
	ColorSpecular,
	NormalSpecular,
	Depth,
	Light
}; };


//////////////////////////////////////////////////////////////////////////
// Renderer keeps a list of object instances to render
//////////////////////////////////////////////////////////////////////////
struct SRenderObject
{
	bool			render;				// should the object be rendered this frame?
	float			screenZ;					// z depth of the object for sorting
	CGeometryGroup	*group;				// geometry group with buffers to render
	CObject3D		*object;			// object instance with transformations, effects etc

	CMaterial *GetMaterial();
};

//////////////////////////////////////////////////////////////////////////
struct SRenderStats
{
	int programs;
	int geometries;
	int textures;
	int calls;
	int vertices;
	int faces;
};

//////////////////////////////////////////////////////////////////////////
struct SRenderTarget
{
	ID3D10Texture2D				*pTexture;
	ID3D10RenderTargetView		*pView;
	ID3D10ShaderResourceView	*pResView;

	SRenderTarget()
	{
		pTexture = NULL;
		pView = NULL;
		pResView = NULL;
	};

	void Release()
	{
		SAFE_DELETE(pTexture);
		SAFE_DELETE(pView);
		SAFE_DELETE(pResView);
	};
};

//////////////////////////////////////////////////////////////////////////
class CRenderer
{
public:
	//Singleton(CRenderer);
	CRenderer();

	// Initialization
	void	Init( ID3D10Device *pDevice );
	void	Release();
	void	Reset();
	
	// Debug rendering
	void AddPoint		( Vector3 p, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddLine		( Vector3 p1, Vector3 p2, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddLines		( Vector3 *points, int num, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddTriangle	( Vector3* v, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddTriangles	( Vector3** v, int num, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddAxis		( Vector3 pos, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddCross		( Vector3 pos, SRGBA color = WHITE, Matrix4 *m = NULL );
	void AddBBox		( SBBox bbox, Vector3 pos, SRGBA color = WHITE, Matrix4 *m = NULL );
	void Addsphere		( Vector3 center, float radius, SRGBA color = WHITE);

	void			RenderDebugPrimitives(CCamera& camera);
	void			Render(CScene &scene, CCamera &camera, void *renderTarget = NULL, bool forceClear = false);

	void			SetBlending(EBlending::Type blending);
	void			SetStencilState(bool test, bool write);
	void			SetRasterState(int state);

	void			Clear(bool screen, bool depth, SRGBA color = BLACK);
	void			SetFillMode();
	void			SetWireframeMode();

	void			RemoveSprite(CSprite2D *spr);
	void			AddSprite(CSprite2D *spr);
	CSprite2D*		AddSprite(CTexture *tex, float x, float y, SRGBA color = WHITE, float scaleX = 1.0f, float scaleY = 1.0f);
	CSprite2D*		AddSpriteRect(CTexture *tex, float x, float y, float w, float h, SRGBA color = WHITE);

	
protected:
	void			InitShadowMap();	
	void			ReleaseShadowMap();
	void			InitSkybox();

	void			InitGBuffer();
	void			ClearGBuffer();
	void			ReleaseGBuffer();
	void			RenderGBuffer(SShaderParameters *params);
	void			RenderGBufferDirectionalLights();
	void			RenderGBufferPointLights();
	void			RenderGBufferCombine();
	void			DrawDebugGBuffers();
	void			RenderFinalView();
	void			DrawDebugShadowView();

	void			RenderShadowMap(CScene &scene);
	void			InitObjects(CScene &scene);
	void			AddObject(CObject3D &object, CScene &scene);
	void			RemoveObject(CObject3D &object);
	void			UpdateObject(CObject3D &object);
	void			RenderObjects(CArray<SRenderObject> &objects, bool reverse, bool transparent, CCamera &camera, bool useBlending, SShaderParameters &params);
	void			RenderObject(CGeometryGroup *group, CMesh *mesh, CCamera &camera, CMaterial *material, SShaderParameters &params);
	
	CShader*		SetShader(CCamera &camera, CMaterial *material, CObject3D *mesh, SShaderParameters &params);

	void			InitSprites();
	void			ReleaseSprites();
	void			RenderSprites3D(CArray<CSprite3D*> &sprites, CCamera &camera, SShaderParameters &params);
	void			RenderSprites(CArray<CSprite*> &sprites, CCamera &camera, SShaderParameters &params, bool screen = false);

	void			InitRenderTargets();
	void			ReleaseRenderTargets();
	void			CreateRenderTarget(SRenderTarget &rt);	

	void			RenderHUDSprites();
	void			ReleaseHUDSprites();	

	void			SSAO();
	void			Bloom(ID3D10RenderTargetView *pRenderTarget);
	void			FXAA(ID3D10RenderTargetView *pRenderTarget);

public:
	bool			autoClear;
	bool			autoClearColor;
	bool			autoClearDepth;	
	SRGBA			clearColor;
	bool			sortObjects;

	SRenderStats	stats;

	CMaterial					*pDefaultMaterial;

	ID3D10Texture2D				*pShadowMap;						// shadow map texture
	ID3D10DepthStencilView		*pShadowMapDepthView;				// render target
	ID3D10ShaderResourceView	*pShadowMapRSView;					// shadow map resource view
	CMaterial					*pShadowMapMaterial;
	D3D10_VIEWPORT				shadowMapViewport;

	bool						showBuffers;

	CScene						*pCurrentScene;
	CCamera						*pCurrentCamera;
	CGeometryRenderer			*pGeometryRenderer;
	
private:
	HRESULT			CreateBuffers();
	void			ReleaseBuffers();	
	void			CreateShaders();
	void			ReleaseShaders();

	inline void		SwapPostRenderTargets()
	{
		SRenderTarget *tmp = postRenderTargets[0];
		postRenderTargets[0] = postRenderTargets[1];
		postRenderTargets[1] = tmp;
	}
	
private:
	ID3D10Device	*pDevice;
	ID3D10Buffer	*pPointsBuffer;
	ID3D10Buffer	*pLinesBuffer;
	ID3D10Buffer	*pTrianglesBuffer;	

	SRenderTarget			gBuffers[MAX_GBUFFERS_NUM];
	ID3D10Texture2D			*pGBufferDS;
	ID3D10DepthStencilView	*pGBufferDSView;	

	SRenderTarget		finalRenderTarget;
	SRenderTarget		offBuffer;

	D3D10_VIEWPORT		objectsViewport;			// viewport used for rendering objects (0- ~0.95 range)
	D3D10_VIEWPORT		skyboxViewport;				// viewport for rendering skybox (+0.95)

	CGeometry			*pSphereGeometry;
	CGeometry			*pConeGeometry;
	
	bool				shadowMapEnabled;	

	SVertexColor		debugPoint[MAX_DEBUG_POINTS];
	SVertexColor		debugLines[MAX_DEBUG_LINES*2];
	SVertexColor		debugTriangles[MAX_DEBUG_TRIANGLES*3];

	uint32				debugPointsNum;
	uint32				debugLinesNum;
	uint32				debugTrianglesNum;

	CSpriteMaterial		*pSpriteMaterial;
	ID3D10Buffer		*pSpriteBuffer;	

	EBlending::Type		oldBlending;
	int					oldStencilState;

	CShader				*pCurrentShader;
	int					currentMaterialId;

	CColorShader		*pColorShader;
	CShader				*pBillboardShader;

	CCamera				*pShadowCaster;

	SRenderTarget		*postRenderTargets[2];

	CArray<CSprite2D*>	hudSprites;
};

extern CRenderer gRenderer;
extern CRITICAL_SECTION renderCS;