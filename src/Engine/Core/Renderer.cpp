#include "platform.h"
#include "Renderer.h"
#include "Direct3D.h"
#include "Shaders.h"
#include "Engine.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Geometry.h"
#include "Material.h"
#include "Sprite.h"
#include "Game.h"
#include "CVar.h"
#include "GeometryRenderer.h"
#include "Geometries.h"
#include "Terrain.h"
#include "FrameGraph.h"
#include "Utils.h"
#include "Light.h"

CRenderer gRenderer;

CRITICAL_SECTION renderCS;

CVar cv_drawBuffers("drawBuffers", "0", FCVAR_ARCHIVE);
CVar cv_shadowNear = CVar("shadowNear", "1", FCVAR_ARCHIVE);
CVar cv_shadowFar = CVar("shadowFar", "500", FCVAR_ARCHIVE);

//////////////////////////////////////////////////////////////////////////
int FrontToBackSort(const SRenderObject &a, const SRenderObject &b)
{
	return b.screenZ > a.screenZ;
}

int BackToFrontSort(const SRenderObject &a, const SRenderObject &b)
{
	return b.screenZ < a.screenZ;
}

int SpriteSort(const CSprite *a, const CSprite *b)
{
	if (!a || !b) return 0;
	return b->screenZ < a->screenZ;
}

int Sprite3DSort(const CSprite3D *a, const CSprite3D *b)
{
	if (!a || !b) return 0;
	return b->screenZ < a->screenZ;
}

CMaterial* GetParentMaterial(CObject3D *obj)
{
	if (obj->material) return obj->material;
	if (obj->parent) return GetParentMaterial(obj->parent);
	return gRenderer.pDefaultMaterial;
}

int SpriteBackToFrontSort(const CSprite2D *a, const CSprite2D *b)
{
	return b->z > a->z;
}

//////////////////////////////////////////////////////////////////////////
CMaterial* SRenderObject::GetMaterial() 
{
	if (object->material) return object->material;		// object material override	
	if (group->materialIndex >= 0 && object->geometry ) 
	{		
		if (object->geometry->materials.Size() > group->materialIndex)			// use geometry material if exists
			return object->geometry->materials[group->materialIndex];

		// otherwise try to use mesh'es material
		return GetParentMaterial(object);
	}

	return GetParentMaterial(object->parent);
}

//////////////////////////////////////////////////////////////////////////
CRenderer::CRenderer()
{
	pPointsBuffer = NULL;
	pLinesBuffer = NULL;
	pTrianglesBuffer = NULL;
	debugPointsNum = 0;
	debugLinesNum = 0;
	debugTrianglesNum = 0;
	pColorShader = NULL;


	autoClear = true;
	autoClearColor = true;
	autoClearDepth = true;
	autoClearDepth = true;
	sortObjects = true;
	clearColor = SRGBA(33,33,33,255);

	oldStencilState = OM_DEPTH_TEST | OM_DEPTH_WRITE;
	oldBlending = EBlending::Normal;

	pCurrentShader = NULL;
	currentMaterialId = -1;
	pDefaultMaterial = NULL;
	showBuffers = true;

	shadowMapEnabled = true;
	pShadowCaster = NULL;
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Init( ID3D10Device *pDevice )
{
	trace("Initializing Renderer");
	this->pDevice = pDevice;
	CreateBuffers();
	CreateShaders();
	InitSprites();
	InitShadowMap();
	InitGBuffer();
	InitRenderTargets();

	InitSkybox();	

	pDefaultMaterial = new CMaterial();
	pDefaultMaterial->features = EShaderFeature::LIGHT | EShaderFeature::FOG | EShaderFeature::SHADOW | EShaderFeature::TEXTURE;	
	pDefaultMaterial->specular = 0.01f;
	pDefaultMaterial->specularPower = 0.1f;

	pGeometryRenderer = new CGeometryRenderer();
	pGeometryRenderer->Init();	

	InitializeCriticalSection(&renderCS);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::InitSkybox()
{
	skyboxViewport.TopLeftX = objectsViewport.TopLeftX = 0;
	skyboxViewport.TopLeftY = objectsViewport.TopLeftY = 0;
	skyboxViewport.Width = objectsViewport.Width = gEngine.GetWidth();
	skyboxViewport.Height = objectsViewport.Height = gEngine.GetHeight();
	objectsViewport.MinDepth = 0.0f;
	objectsViewport.MaxDepth = 0.9999f;
	skyboxViewport.MinDepth = objectsViewport.MaxDepth + 0.00001;
	skyboxViewport.MaxDepth = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Release()
{
	ReleaseBuffers();
	ReleaseShaders();
	ReleaseSprites();
	ReleaseShadowMap();
	ReleaseGBuffer();
	ReleaseHUDSprites();
	ReleaseRenderTargets();

	SAFE_RELEASE(pGeometryRenderer);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CRenderer::CreateBuffers()
{
	trace("Creating Debug Buffers");
	D3D10_BUFFER_DESC desc;
	desc.Usage = D3D10_USAGE_DYNAMIC;
	desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	desc.ByteWidth = sizeof(SVertexColor) * 1 * MAX_DEBUG_POINTS;
	V_RETURN( pDevice->CreateBuffer( &desc, NULL, &pPointsBuffer ) );

	desc.ByteWidth = sizeof(SVertexColor) * 2 * MAX_DEBUG_LINES;
	V_RETURN( pDevice->CreateBuffer( &desc, NULL, &pLinesBuffer ) );

	desc.ByteWidth = sizeof(SVertexColor) * 3 * MAX_DEBUG_TRIANGLES;
	V_RETURN( pDevice->CreateBuffer( &desc, NULL, &pTrianglesBuffer ) );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SetFillMode()
{
	Direct3D::Instance()->SetFillState();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SetWireframeMode()
{
	Direct3D::Instance()->SetWireframeState();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseBuffers()
{
	trace("Releasing Debug Buffers");
	SAFE_RELEASE( pPointsBuffer );
	SAFE_RELEASE( pLinesBuffer );
	SAFE_RELEASE( pTrianglesBuffer );
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderDebugPrimitives(CCamera& camera)
{
	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	SetStencilState(true, true);
	SetWireframeMode();

	uint32 stride = sizeof(SVertexColor);
	uint32 offset = 0;
	pDevice->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	
	D3DXMATRIX matView, matWorld, matProjection;
		
	matProjection = camera.matrixProjection;
	matView = camera.matrixView;
	
	D3DXMatrixIdentity( &matWorld );	// REQUIRED

	pColorShader->pProjectionMatrix->SetMatrix((float*)&matProjection);
	pColorShader->pWorldMatrix->SetMatrix((float*)&matWorld);
	pColorShader->pViewMatrix->SetMatrix((float*)&matView);

	// Copy the buffers
	SVertexColor *pData = NULL;
	if ( SUCCEEDED( pPointsBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, debugPoint, sizeof(SVertexColor) * debugPointsNum );
		pPointsBuffer->Unmap();
	}

	HRESULT result;
	if ( SUCCEEDED( result = pLinesBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, debugLines, sizeof(SVertexColor) * debugLinesNum * 2 );
		pLinesBuffer->Unmap();
	}
	
	if ( SUCCEEDED( pTrianglesBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, debugTriangles, sizeof(SVertexColor) * debugTrianglesNum * 3 );
		pTrianglesBuffer->Unmap();
	}

	// Points
	if (debugPointsNum > 0)
	{		
		pDevice->IASetVertexBuffers(0, 1, &pPointsBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
		pColorShader->Render(debugPointsNum);
		debugPointsNum = 0;
	}	

	// Lines
	if (debugLinesNum > 0)
	{
		pDevice->IASetVertexBuffers(0, 1, &pLinesBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		pColorShader->Render(debugLinesNum*2);
		debugLinesNum = 0;
	}

	// Triangles
	if (debugTrianglesNum > 0)
	{
		pDevice->IASetVertexBuffers(0, 1, &pTrianglesBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pColorShader->Render(debugTrianglesNum*3);
		debugTrianglesNum = 0;
	}

	SetFillMode();
	gD3D->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseShaders()
{
	trace("Releasing Shaders");
	SAFE_RELEASE(pColorShader);
	SAFE_RELEASE(pBillboardShader);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::CreateShaders()
{
	trace("Initializing Shaders");
	pColorShader = new CColorShader();
	pColorShader->Init(pDevice);
	pBillboardShader = new CShader("shadowmap_debug.fx");
	pBillboardShader->Create(CShaderFactory::GetShaderCode("shadowmap_debug.fx"), pDevice);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::AddPoint( Vector3 p, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ ) 
{
	if (debugPointsNum == MAX_DEBUG_POINTS) return;
	if (m) p = m->MultiplyVector3(p);		// if supllied transformation matrix
	SVertexColor v = { p.x, p.y, p.z, color.ToInt32() };
	debugPoint[debugPointsNum++] = v;
}

void CRenderer::AddLine( Vector3 p1, Vector3 p2, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{
	if (debugLinesNum == MAX_DEBUG_LINES) return;
	if (m) 
	{
		p1 = m->MultiplyVector3(p1);		// if supllied transformation matrix
		p2 = m->MultiplyVector3(p2);		// if supllied transformation matrix
	}
	SVertexColor v1 = { p1.x, p1.y, p1.z, (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f   };
	SVertexColor v2 = { p2.x, p2.y, p2.z, (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f };
	debugLines[debugLinesNum*2+0] = v1;
	debugLines[debugLinesNum*2+1] = v2;
	debugLinesNum++;
}

void CRenderer::AddLines( Vector3 *points, int num, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{
	if (debugLinesNum + num > MAX_DEBUG_LINES) num -= debugLinesNum + num - MAX_DEBUG_LINES;
	for (int i=0; i<num; ++i)	
	{
		if (m) 
		{
			if (i == 0) points[0] = m->MultiplyVector3(points[0]);	
			points[i*2+1] = m->MultiplyVector3(points[i*2+1]);	
		}
		AddLine( points[i*2+0], points[i*2+1], color );	
	}
}

void CRenderer::AddTriangle	( Vector3* v, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{
	if (debugTrianglesNum == MAX_DEBUG_TRIANGLES) return;
	for (int i=0; i<3; ++i)
	{
		if (m) v[i] = m->MultiplyVector3(v[i]);
		SVertexColor vert = { v[i].x, v[i].y, v[i].z, (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f };
		debugTriangles[debugTrianglesNum*3+i] = vert;
	}
	debugTrianglesNum++;
}

void CRenderer::AddTriangles( Vector3** v, int num, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{
	if (debugTrianglesNum + num > MAX_DEBUG_TRIANGLES) num -= debugTrianglesNum + num - MAX_DEBUG_TRIANGLES;
	for (int i=0; i<num; ++i)	
		AddTriangle( v[i], color, m );	
}

void CRenderer::AddAxis( Vector3 pos, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ ) 
{
	AddLine( pos, pos + Vector3(1,0,0), RED, m );
	AddLine( pos, pos + Vector3(0,1,0), BLUE, m );
	AddLine( pos, pos + Vector3(0,0,1), GREEN, m );
}

void CRenderer::AddCross( Vector3 pos, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{	
	AddLine( pos - Vector3(0.5f,0,0), pos + Vector3(0.5f,0,0), color, m );
	AddLine( pos - Vector3(0,0.5f,0), pos + Vector3(0,0.5f,0), color, m );
	AddLine( pos - Vector3(0,0,0.5f), pos + Vector3(0,0,0.5f), color, m );
}

void CRenderer::AddBBox( SBBox bbox, const Vector3 pos, SRGBA color /*= WHITE*/, Matrix4 *m /*= NULL*/ )
{	
	Vector3 min = pos + bbox.min;
	Vector3 max = pos + bbox.max;		
	AddLine( min, Vector3(max.x, min.y, min.z), color, m );
	AddLine( min, Vector3(min.x, max.y, min.z), color, m );
	AddLine( min, Vector3(min.x, min.y, max.z), color, m );
	AddLine( Vector3(max.x, min.y, max.z), Vector3(min.x, min.y, max.z), color, m );
	AddLine( Vector3(max.x, min.y, max.z), Vector3(max.x, min.y, min.z), color, m );
	AddLine( Vector3(max.x, min.y, max.z), Vector3(max.x, max.y, max.z), color, m );
	AddLine( Vector3(min.x, max.y, max.z), max, color, m );
	AddLine( Vector3(min.x, max.y, max.z), Vector3(min.x, max.y, min.z), color, m );
	AddLine( Vector3(min.x, max.y, max.z), Vector3(min.x, min.y, max.z), color, m );
	AddLine( Vector3(max.x, max.y, min.z), Vector3(min.x, max.y, min.z), color, m );
	AddLine( Vector3(max.x, max.y, min.z), max, color, m );
	AddLine( Vector3(max.x, max.y, min.z), Vector3(max.x, min.y, min.z), color, m );
}

//////////////////////////////////////////////////////////////////////////
// Main rendering function
//////////////////////////////////////////////////////////////////////////
void CRenderer::Render( CScene &scene, CCamera &camera, void *renderTarget /*= NULL*/, bool forceClear /*= false*/ )
{
	EnterCriticalSection(&renderCS);

	gFrameGraph.Begin("Scene Update", BLUE);
	pCurrentScene = &scene;
	pCurrentCamera = &camera;

	postRenderTargets[0] = &finalRenderTarget;
	postRenderTargets[1] = &offBuffer;

	// make sure camera is added to scene
	if (camera.parent == NULL) scene.Add(&camera);

	// initialize newly added objects, remove flagged for destruction
	scene.InitObjects();

	// calculate world matrices of all scene objects 
	scene.UpdateMatrixWorld();

	gFrameGraph.End("Scene Update");
	
	// Reset rendering stats
	stats.calls = stats.vertices = stats.faces = 0;

	// Calculate camera frustum	
	camera.matrixView.GetInverse(camera.matrixWorld);		
	camera.matrixProjectionScreen = Matrix4::Multiply(camera.matrixProjection, camera.matrixView);
	camera.CalculateFrustum();

	//Render Shadow Map
	if (shadowMapEnabled) 
	{
		gFrameGraph.Begin("Shadowmap", SRGB(255,255,128));
		RenderShadowMap(scene);
		gFrameGraph.End("Shadowmap");
	}

	// clear buffers
	if (autoClear || forceClear) 
		gD3D->Clear(autoClearColor, autoClearDepth, clearColor);

	// setup objects' matrices
	gFrameGraph.Begin("Set Matrices", SRGB(255,128,128));
	scene.SetupMatrices(camera);	
	gFrameGraph.End("Set Matrices");

	// render debug geometry
	SetStencilState(true, true);

	// sort objects by Z from back to front
	if (sortObjects)
		std::sort(scene.renderObjects.m_Memory.m_pMemory,scene.renderObjects.m_Memory.m_pMemory + scene.renderObjects.Size(), FrontToBackSort);

		
	SetBlending(EBlending::Normal);

	// create render parameters
	SShaderParameters params; 
	params.fog = scene.fog;
	params.lights = &scene.lights;
	params.ambientColor = &scene.ambientColor;
	params.shadowPass = false;

	gD3D->SetRasterState(RS_CULL);


	// G-Buffer rendering	
	ClearGBuffer();								// clear G-Buffer	

	gFrameGraph.Begin("G-Buffer Render", SRGB(255,0,128));
	RenderGBuffer(&params);						// render g-buffer contents
	gFrameGraph.End("G-Buffer Render");

	// Draw SSAO after rendering the geometry and before lighting and particles
	SSAO();

	gFrameGraph.Begin("G-Buffer DirLights", SRGB(255,128,0));
	RenderGBufferDirectionalLights();
	gFrameGraph.End("G-Buffer DirLights");

	gFrameGraph.Begin("G-Buffer Lights", SRGB(128,255,0));
	RenderGBufferPointLights();
	gFrameGraph.End("G-Buffer Lights");


	Clear(false, true);

	gFrameGraph.Begin("G-Buffer Combine", SRGB(0,255,128));
	RenderGBufferCombine();	
	gFrameGraph.End("G-Buffer Combine");

	// Render sprites
	currentMaterialId = -1;
	pCurrentShader = NULL;
	
	
	gFrameGraph.Begin("Sprites3D", SRGB(0,255,255));
	RenderSprites3D(scene.sprites3d, camera, params);
	gFrameGraph.End("Sprites3D");

	gFrameGraph.Begin("Sprites", SRGB(0,128,255));
	RenderSprites(scene.sprites, camera, params);
	gFrameGraph.End("Sprites");

	SetBlending(EBlending::Normal);

	// Debug geometry
	gFrameGraph.Begin("Debug", SRGB(0,128,0));
	RenderDebugPrimitives(camera);
	gFrameGraph.End("Debug");

	Bloom();
	FXAA();
	
	SetBlending(EBlending::Normal);

	RenderFinalView();

	LeaveCriticalSection(&renderCS);
	
	// draw gbuffer preview	
	if (cv_drawBuffers.GetBool())
	{
		DrawDebugGBuffers();
		DrawDebugShadowView();
	}

	RenderHUDSprites();

	pGeometryRenderer->SetUseAlpha(true);
	SetBlending(EBlending::Normal);
	SetStencilState(false, false);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::DrawDebugGBuffers()
{
	float size = 0.5f;
	float tu = (float)gEngine.width / 2048.0f;
	float tv = (float)gEngine.height / 2048.0f;
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, -1.0f + size, 0, 1, 1, 1, 1, 0, 0 },
		{ -1.0f + size, -1.0f + size, 0, 1, 1, 1, 1, tu, 0 },
		{ -1.0f + size, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	Clear(false, true);
	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(false);
	SetStencilState(false, false);

	// add all three buffers 
	for (int i = 0; i < 4; i++)
	{
		pGeometryRenderer->SetTexture(gBuffers[i].pResView);
		pGeometryRenderer->DrawQuad(v);
		pGeometryRenderer->Render();
		v[0].x += size;
		v[1].x += size;
		v[2].x += size;
		v[3].x += size;
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::DrawDebugShadowView()
{
	float size = 0.5f;
	float tu = 1.0;
	float tv = 1.0;
	SVertexColorTextured v[4] = {
		{ -1, 1 - size, 0,						1, 1, 1, 1, 0, tv },
		{ -1, 1, 0,				1, 1, 1, 1, 0, 0 },
		{ -1.0f + size, 1, 0,	1, 1, 1, 1, tu, 0 },
		{ -1.0f + size, 1-size, 0,				1, 1, 1, 1, tu, tv }
	};

	Clear(false, true);
	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(false);
	SetStencilState(false, false);

	pGeometryRenderer->SetTexture(pShadowMapRSView);
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Clear(bool screen, bool depth, SRGBA color /* = BLACK */)
{
	gD3D->Clear(screen, depth, color);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SetBlending( EBlending::Type blending )
{
	if (blending == oldBlending) return;	
	gD3D->SetBlending(blending);	
	oldBlending = blending;
}

//////////////////////////////////////////////////////////////////////////
// Render given set of render objects
//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderObjects( CArray<SRenderObject> &objects, bool reverse, bool transparent, CCamera &camera, bool useBlending, SShaderParameters &params )
{
	int start = 0, end = objects.Size(), delta = 1;

	pCurrentShader = NULL;

	if (reverse) 
	{
		start = objects.Size()-1;
		end = -1;
		delta = -1;
	}

	for (int i=start; i!=end; i+=delta)
	{
		SRenderObject &ro = objects[i];
		if (!ro.render) continue;

		CMesh *mesh = (CMesh*)ro.object;
		if (!mesh->castShadow) continue;

		CMaterial *material = params.shadowPass ? pShadowMapMaterial : ro.GetMaterial();

		if (material->transparent != transparent) continue;		// this pass is either for opaque or transparent

		SetBlending(material->blending);
		SetStencilState(material->depthTest, (material->blending==EBlending::Additive ? false : material->depthWrite));


		int state = 0;
		if (material->wireframe) state |= RS_WIREFRAME;
		if (!material->doubleSided) state |= RS_CULL;

		Direct3D::Instance()->SetRasterState(state);
		
		RenderObject(ro.group, mesh, camera, material, params);
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SetStencilState( bool test, bool write )
{
	int state = 0;
	if (test) state |= OM_DEPTH_TEST;
	if (write) state |= OM_DEPTH_WRITE;
	if (state == oldBlending) return;
	gD3D->SetStencilState(test, write);
	oldStencilState = state;
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SetRasterState(int state)
{
	Direct3D::Instance()->SetRasterState(state);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderObject( CGeometryGroup *group, CMesh *mesh, CCamera &camera, CMaterial *material, SShaderParameters &params )
{
	CShader *shader = SetShader(camera, material, mesh, params);

	uint32 stride = group->vertexSize * sizeof(float);
	uint32 offset = 0;
	
	pDevice->IASetVertexBuffers(0, 1, &group->pVertexBuffer, &stride, &offset);	
	pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D10_TECHNIQUE_DESC techniqueDesc;
	shader->pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		shader->pTechnique->GetPassByIndex(i)->Apply(0);
		pDevice->Draw(group->vertexCount, 0);
	}

	stats.calls++;
}

//////////////////////////////////////////////////////////////////////////
// Initialize material if first time used since creation
// Setup shader uniforms if switched to new shader
//////////////////////////////////////////////////////////////////////////
CShader* CRenderer::SetShader( CCamera &camera, CMaterial *material, CObject3D *mesh, SShaderParameters &params )
{	
	// create shader for this material (or reuse existing one)
	if (!material->pShader)	CShaderFactory::InitMaterial(material, params);

	CShader *shader = material->pShader;
	bool refresh = (shader != pCurrentShader);		// should the shader be initialized?
	if (refresh)
	{
		pDevice->IASetInputLayout(shader->pLayout);
		pCurrentShader = shader;
		shader->SetGlobalUniforms(&camera, &params);		// set global uniforms, once per shader program
	}

	if (material->id != currentMaterialId)
	{
		currentMaterialId = material->id;
		shader->SetMaterialUniforms(material);				// set material uniforms, per each material change
	}

	if (mesh)
	{
		shader->SetObjectUniforms(mesh, material);			// set object uniforms, per each object
	}

	return shader;
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::InitSprites()
{	
	D3D10_BUFFER_DESC desc;
	desc.Usage = D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.ByteWidth = 6 * 5 * sizeof(float);

	float pVertexArray[] = { 
		-0.5f, -0.5f, 0,	0, 0,		+0.5f, -0.5f, 0,	1, 0,		+0.5f, +0.5f, 0,	1, 1,
		-0.5f, -0.5f, 0,	0, 0,		+0.5f, +0.5f, 0,	1, 1,		-0.5f, +0.5f, 0,	0, 1
	};
	
	D3D10_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, 0, sizeof(initData));
	initData.pSysMem = pVertexArray;

	HRESULT hr;
	if ( FAILED( hr = pDevice->CreateBuffer( &desc, &initData, &pSpriteBuffer )) )
	{		
		trace("ERROR: %d",hr);
	}

	pSpriteMaterial = new CSpriteMaterial();	
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseSprites()
{
	SAFE_RELEASE(pSpriteBuffer);
	SAFE_DELETE(pSpriteMaterial);		
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderSprites3D( CArray<CSprite3D*> &sprites, CCamera &camera, SShaderParameters &params )
{
	CShader *shader = NULL;
	uint32 stride = 5 * sizeof(float);
	uint32 offset = 0;

	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	SetStencilState(true, true);
	
	// sort sprites from back to front	
	for (int i=0; i<sprites.Size(); i++)
	{
		Vector3 v = sprites[i]->GetPosition();
		camera.matrixView.MultiplyVector3(v);
		sprites[i]->screenZ = v.z;				
	}
	
	std::sort(sprites.m_Memory.m_pMemory, sprites.m_Memory.m_pMemory + sprites.Size(), Sprite3DSort);
		
	pDevice->IASetVertexBuffers(0, 1, &pSpriteBuffer, &stride, &offset);	
	pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	// draw sprites one by one
	for (int i=0; i<sprites.Size(); i++)
	{
		CSprite3D *spr = sprites[i];

		pSpriteMaterial->pTexture = spr->pTexture;
		shader = SetShader(camera, pSpriteMaterial, sprites[i], params);
		
		shader->uniforms["diffuseTexture"].SetResource(spr->pTexture ? spr->pTexture->pTextureView : NULL);
		shader->uniforms["position"].Set(spr->position);
		shader->uniforms["rotation"].Set(spr->rotation.x);
		shader->uniforms["alignment"].Set(Vector2(0,0));	// TODO
		shader->uniforms["size"].Set( Vector2(spr->size.x * spr->scale.x, spr->size.y * spr->scale.y) );
		shader->uniforms["screenPosition"].Set(false);		
		shader->uniforms["locked"].Set(spr->locked);
		if (spr->locked)
			shader->uniforms["lockedAxis"] = spr->lockedAxis;
		shader->uniforms["diffuse"] = spr->color;
		

		Vector4 texcoords = Vector4(spr->texcoord[0].x, spr->texcoord[0].y, spr->texcoord[1].x, spr->texcoord[1].y);

		if (spr->pTexture)
			shader->uniforms["texCoords"] = texcoords;

		gRenderer.SetBlending(spr->additive ? EBlending::Additive : EBlending::Normal);		
		gRenderer.SetStencilState(spr->depthRead, true);

		D3D10_TECHNIQUE_DESC techniqueDesc;
		shader->pTechnique->GetDesc(&techniqueDesc);

		for (int i = 0; i < techniqueDesc.Passes; i++)
		{
			shader->pTechnique->GetPassByIndex(i)->Apply(0);
			pDevice->Draw(6, 0);
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderHUDSprites()
{
	CShader *shader = CShaderFactory::GetShader("hudSprites.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	pDevice->IASetInputLayout(shader->pLayout);

	SVertexColorTextured v[4] = {
		{ -0.5f, +0.5f, 0, 1, 1, 1, 1, 0, 0 },
		{ -0.5f, -0.5f, 0, 1, 1, 1, 1, 0, 1 },
		{ +0.5f, -0.5f, 0, 1, 1, 1, 1, 1, 1 },
		{ +0.5f, +0.5f, 0, 1, 1, 1, 1, 1, 0 }
	};

	pDevice->OMSetRenderTargets(1, &gD3D->pRenderTargetView, gD3D->pDepthStencilView);
	SetStencilState(false, true);
	SetBlending(EBlending::Normal);	
	gD3D->SetRasterState(0);	

	// sort objects by Z from back to front
	std::sort(hudSprites.m_Memory.m_pMemory,hudSprites.m_Memory.m_pMemory + hudSprites.Size(), SpriteBackToFrontSort);


	pGeometryRenderer->Reset();

	shader->uniforms["screenWidth"] = gEngine.width;
	shader->uniforms["screenHeight"] = gEngine.height;

	// draw sprites one by one
	for (int i=0; i<hudSprites.Size(); i++)
	{
		CSprite2D *spr = hudSprites[i];
		if (!spr->visible) continue;
		
		shader->uniforms["noTexture"].Set(spr->pTexture == NULL);
		if (spr->pTexture != NULL)
			shader->uniforms["diffuseTexture"].SetResource(spr->pTexture->pTextureView);
		shader->uniforms["position"].Set(spr->position);
		shader->uniforms["rotation"].Set(spr->rotation);
		shader->uniforms["offset"].Set(spr->offset);		

		if (spr->size.x > 0 && spr->size.y)
			shader->uniforms["size"].Set(spr->size);		
		else
			shader->uniforms["size"].Set( Vector2( (float)spr->pTexture->width * spr->scale.x, (float)spr->pTexture->height * spr->scale.y) );		

		shader->uniforms["color"] = spr->color;

		Vector4 texcoords = Vector4(spr->texCoords[0].x, spr->texCoords[0].y, spr->texCoords[1].x, spr->texCoords[1].y);
		shader->uniforms["texCoords"] = texcoords;
		
		pGeometryRenderer->DrawQuad(v);		
		pGeometryRenderer->Render(shader);
	}		
}


//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderSprites(CArray<CSprite*> &sprites, CCamera &camera, SShaderParameters &params, bool screen)
{
	// set global and sprite material uniforms
	CShader *shader = SetShader(camera, pSpriteMaterial, NULL, params);
	uint32 stride = 5 * sizeof(float);
	uint32 offset = 0;

	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	SetStencilState(true, true);

	// sort sprites from back to front, also, if they are attached to parent objects, set their position to those
	// but only if not screen sprites
	if (!screen)
	{		
		for (int i=0; i<sprites.Size(); i++)
		{
			CSprite *s = sprites[i];
			
			if (s->pParent != NULL)
			{
				// if parent is removed, remove this sprite aswell
				if (s->pParent->deleteMe)
				{
					Debug("Sprite's %d parent is removed, removing sprite as well", i);
					sprites.RemoveAt(i--);
					continue;
				}

				// if sprite has a prent, use it's position instead
				s->position = s->pParent->GetWorldPosition();
			}

			Vector3 v = sprites[i]->position;
			camera.matrixView.MultiplyVector3(v);
			sprites[i]->screenZ = v.z;				
		}

		std::sort(sprites.m_Memory.m_pMemory, sprites.m_Memory.m_pMemory + sprites.Size(), SpriteSort);
	}

	// set up layout and topology for rendering
	pDevice->IASetVertexBuffers(0, 1, &pSpriteBuffer, &stride, &offset);	
	pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// render sprite geometry
	D3D10_TECHNIQUE_DESC techniqueDesc;
	shader->pTechnique->GetDesc(&techniqueDesc);

	shader->mFlags.Set(MF_DIFFUSE_TEXTURE);

	// draw sprites one by one
	for (int i=0; i<sprites.Size(); i++)
	{
		CSprite *spr = sprites[i];

		// create world position matrix
		Matrix4 matrix;
		//matrix.SetPosition(spr->position);
		shader->worldMatrix.Set(matrix);

		// set texture and other parameters
		shader->mDiffuseTexture.Set(spr->pTexture);
		shader->position.Set(spr->position);
		shader->rotation.Set(spr->rotation);
		shader->alignment.Set(spr->alignment);
		shader->size.Set(spr->size);
		shader->screenPosition.Set(false);		
		shader->locked.Set(spr->locked);
		if (spr->locked)
			shader->lockedAxis.Set(spr->lockedAxis);
		shader->diffuse.Set(spr->color);
		
		// texture coordinates
		Vector4 texcoords = Vector4(spr->texcoord[0].x, spr->texcoord[0].y, spr->texcoord[1].x, spr->texcoord[1].y);
		shader->texCoords.Set(texcoords);

		// set render state
		gRenderer.SetBlending(spr->additive ? EBlending::Additive : EBlending::Normal);		
		gRenderer.SetStencilState(spr->depthRead, spr->depthWrite);
		
		for (int i = 0; i < techniqueDesc.Passes; i++)
		{
			shader->pTechnique->GetPassByIndex(i)->Apply(0);
			pDevice->Draw(6, 0);
		}
	}		
}

//////////////////////////////////////////////////////////////////////////
// Initialize G-Buffer render targets
//////////////////////////////////////////////////////////////////////////
void CRenderer::InitGBuffer()
{
	trace("Creating G-Buffer Render Target Views");
	int width = 2048;// gEngine.GetWidth();
	int height = 2048;// gEngine.GetHeight();

	// texture definition for render targets
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;		// cannot be depth stencil
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// render target view description
	D3D10_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	// shader resource view for the textures
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	// create depth stencil view as well
	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	// texture definition for depth stencil view
	D3D10_TEXTURE2D_DESC dsvTexDesc;
	ZeroMemory(&dsvTexDesc, sizeof(dsvTexDesc));
	dsvTexDesc.Width = width;
	dsvTexDesc.Height = height;
	dsvTexDesc.MipLevels = 1;
	dsvTexDesc.ArraySize = 1;
	dsvTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsvTexDesc.SampleDesc.Count = 1;
	dsvTexDesc.SampleDesc.Quality = 0;
	dsvTexDesc.Usage = D3D10_USAGE_DEFAULT;
	dsvTexDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	dsvTexDesc.CPUAccessFlags = 0;
	dsvTexDesc.MiscFlags = 0;
	
	DXV( pDevice->CreateTexture2D( &dsvTexDesc, NULL, &pGBufferDS ) );
	DXV( pDevice->CreateDepthStencilView( pGBufferDS, &dsvDesc, &pGBufferDSView ) );
	
	// create all render target views
	for (int i=0; i<MAX_GBUFFERS_NUM; i++)
	{
		DXV( pDevice->CreateTexture2D( &texDesc, NULL, &gBuffers[i].pTexture ));
		DXV( pDevice->CreateRenderTargetView( gBuffers[i].pTexture, &rtvDesc, &gBuffers[i].pView ));
		DXV( pDevice->CreateShaderResourceView( gBuffers[i].pTexture, &srvDesc, &gBuffers[i].pResView ) );		
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::InitRenderTargets()
{
	// texture definition for render targets
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = 2048;
	texDesc.Height = 2048;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;		// cannot be depth stencil
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// render target view description
	D3D10_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	// shader resource view for the textures
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;	

	DXV( pDevice->CreateTexture2D( &texDesc, NULL, &finalRenderTarget.pTexture));
	DXV( pDevice->CreateRenderTargetView(finalRenderTarget.pTexture, &rtvDesc, &finalRenderTarget.pView));
	DXV( pDevice->CreateShaderResourceView(finalRenderTarget.pTexture, &srvDesc, &finalRenderTarget.pResView) );

	// create off buffer
	CreateRenderTarget(offBuffer);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::CreateRenderTarget(SRenderTarget &rt)
{
	// texture definition for render targets
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = 2048;
	texDesc.Height = 2048;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;		// cannot be depth stencil
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// render target view description
	D3D10_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	// shader resource view for the textures
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;	

	DXV( pDevice->CreateTexture2D( &texDesc, NULL, &rt.pTexture ));
	DXV( pDevice->CreateRenderTargetView( rt.pTexture, &rtvDesc, &rt.pView ));
	DXV( pDevice->CreateShaderResourceView( rt.pTexture, &srvDesc, &rt.pResView ) );	
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseRenderTargets()
{
	
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseGBuffer()
{
	for (int i=0; i<MAX_GBUFFERS_NUM; i++)
	{
		SAFE_RELEASE( gBuffers[i].pTexture );
		SAFE_RELEASE( gBuffers[i].pView );
		SAFE_RELEASE( gBuffers[i].pResView );
	}
	SAFE_RELEASE(pGBufferDSView);
	SAFE_RELEASE(pGBufferDS);

	// remove g-buffer geometry
	SAFE_DELETE(pSphereGeometry);
	SAFE_DELETE(pConeGeometry);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::InitShadowMap()
{
	trace("Creating Shadow Map");

	// viewport
	shadowMapViewport.TopLeftX = 0;
	shadowMapViewport.TopLeftY = 0;
	shadowMapViewport.MinDepth = 0.0f;
	shadowMapViewport.MaxDepth = 1.0f;
	shadowMapViewport.Width = 2048;// gEngine.GetWidth() * 2;
	shadowMapViewport.Height = 2048;// gEngine.GetHeight() * 2;

	// shadow map texture
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = shadowMapViewport.Width;
	texDesc.Height = shadowMapViewport.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;			// store z-depth component only
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// depth stencil view - allow saving of the depth stencil buffer back to the texture
	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	// depth stencil resource view for binding to the shader
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	DXV( pDevice->CreateTexture2D( &texDesc, NULL, &pShadowMap ) );
	DXV( pDevice->CreateDepthStencilView( pShadowMap, &dsvDesc, &pShadowMapDepthView ) );
	DXV( pDevice->CreateShaderResourceView( pShadowMap, &srvDesc, &pShadowMapRSView ) );

	// depth material
	pShadowMapMaterial = new CDepthMaterial();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseShadowMap()
{
	SAFE_RELEASE( pShadowMap );
	SAFE_RELEASE( pShadowMapDepthView );
	SAFE_RELEASE( pShadowMapRSView );
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderShadowMap(CScene &scene)
{
	// from now on this is our render target
	pDevice->OMSetRenderTargets(0, 0, pShadowMapDepthView);
	pDevice->RSSetViewports(1, &shadowMapViewport);
	pDevice->ClearDepthStencilView(pShadowMapDepthView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	// shadow shader
	CShader *shader = CShaderFactory::GetShader("shadow.fx", EShaderAttribute::POSITION | EShaderAttribute::NORMAL | EShaderAttribute::TEXCOORD);

	// set rendering parameters
	SetBlending(EBlending::Normal);
	SetStencilState(true, true);

	int culled = 0;

	// find lights that cast shadows
	for (int i=0; i<scene.lights.Size(); i++)
	{
		CLight *light = scene.lights[i];
		if (!light->castShadow) continue;	

		Matrix4 view = light->GetViewMatrix();				// view matrix of the light
		Matrix4 projection = light->GetProjectionMatrix();	// light projection matrix	

		// build frustum for this light
		CFrustum frustum;
		frustum.Construct(1000.0f, Matrix4::Multiply(projection, view)); 

		// set shader uniforms		
		shader->viewMatrix.Set(view);
		shader->projectionMatrix.Set(projection);
		shader->uniforms["near"] = light->shadowNear;
		shader->uniforms["far"] = light->shadowFar;

		// draw objects
		int start = 0, end = scene.renderObjects.Size(), delta = 1;
		for (int i=start; i!=end; i+=delta)
		{
			SRenderObject &ro = scene.renderObjects[i];
			if (!ro.render) continue;
			if (!ro.object->castShadow)	continue;
			/*if (!frustum.CheckMesh(*ro.object))		// TODO: Fix frustum
			{
				culled++;
				continue;
			}*/

			// set per-object shader uniforms
			Matrix4 &world = ro.object->matrixWorld;
			shader->worldMatrix.Set(world);

			auto material = ro.object->material;

			int flags = 0;
			CTexture *texture = nullptr;

			int state = 0;
			if (!material || !material->doubleSided) state |= RS_CULL;
			Direct3D::Instance()->SetRasterState(state);

			if (material && material->pTexture)
			{
				texture = material->pTexture;
				flags |= MF_DIFFUSE_TEXTURE;				
			}

			if (texture)
				shader->uniforms["mDiffuseTexture"].Set(*texture);
			else
				shader->uniforms["mDiffuseTexture"].Set(false);
			shader->uniforms["mFlags"].Set(flags);

			pDevice->IASetInputLayout(shader->pLayout);
			shader->DrawGroup(ro.group);
		}
	}
	
	// default render target
	gD3D->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ClearGBuffer()
{
	CShader *shader = CShaderFactory::GetShader("clearGBuffer.fx", EShaderAttribute::POSITION);
	pDevice->IASetInputLayout(shader->pLayout);

	ID3D10RenderTargetView *rtvArray[3] = {
		gBuffers[0].pView, 
		gBuffers[1].pView, 
		gBuffers[2].pView
	};

	pDevice->OMSetRenderTargets(3, rtvArray, pGBufferDSView);
	pDevice->ClearDepthStencilView(pGBufferDSView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	float clear[][4] = {
		{ 0, 0, 0, 0 },
		{ 0.5f, 0.5f, 0.5f, 0 },
		{ 1, 1, 1, 1 }
	};
	pDevice->ClearRenderTargetView(gBuffers[0].pView, clear[0]);
	pDevice->ClearRenderTargetView(gBuffers[1].pView, clear[1]);
	pDevice->ClearRenderTargetView(gBuffers[2].pView, clear[2]);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderGBuffer(SShaderParameters *params)
{
	CScene &scene = *pCurrentScene;
	CCamera &camera = *pCurrentCamera;

	pDevice->ClearDepthStencilView(pGBufferDSView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	pDevice->RSSetViewports(1, &objectsViewport);

	// gbuffer render shader
	CShader *shader = CShaderFactory::GetShader("renderGBuffer.fx", EShaderAttribute::POSITION | EShaderAttribute::NORMAL | EShaderAttribute::TEXCOORD | EShaderAttribute::TANGENT);

	pDevice->IASetInputLayout(shader->pLayout);

	// set rendering parameters
	SetBlending(EBlending::None);
	SetStencilState(true, true);
	SetRasterState(RS_CULL);	
	SetFillMode();

	// set shader uniforms		
	shader->SetGlobalUniforms(&camera, params);
	shader->uniforms["near"].Set(camera.near);
	shader->uniforms["far"].Set(camera.far);

	
	// Terrain rendering code
	shader->uniforms["mTerrainDiffuse"].SetResource( CTexture::Get("textures/terrain3.png")->GetTexture());

	// draw objects
	int start = 0, end = scene.renderObjects.Size(), delta = 1;
	for (int i=start; i!=end; i+=delta)
	{
		SRenderObject &ro = scene.renderObjects[i];
		if (!ro.render) continue;

		// set per-object shader uniforms
		CMaterial *material = ro.GetMaterial();

		int state = 0;
		if (material->wireframe) state |= RS_WIREFRAME;
		if (!material->doubleSided) state |= RS_CULL;
		Direct3D::Instance()->SetRasterState(state);
		
		// set shader variables
		shader->SetMaterialUniforms(material);
		shader->SetObjectUniforms(ro.object, material);		

		// check for terrain
		shader->uniforms["isTerrain"].Set(ro.object->parent == gTerrain.pMesh);
		
		shader->DrawGroup(ro.group);

		stats.calls++;
	}
	

	// draw skybox
	if (scene.pSkybox)
	{
		ID3D10RenderTargetView *rtvArray[1] = {
			gBuffers[0].pView			// draw into the color buffer
		};
		pDevice->OMSetRenderTargets(1, rtvArray, pGBufferDSView);


		// don't write into the z-buffer
		SetStencilState(true, false);

		pDevice->RSSetViewports(1, &skyboxViewport);
		scene.pSkybox->SetPosition(camera.position);	// position the skybox always relative to the camera
		scene.pSkybox->UpdateMatrixWorld(true);

		for (int i=0; i<6; i++)
		{
			CMaterial *material = scene.pSkybox->geometry->materials[i];
			Direct3D::Instance()->SetRasterState(0);
			shader->uniforms["isTerrain"] = false;
			shader->SetMaterialUniforms(material);
			shader->SetObjectUniforms(scene.pSkybox, material);	
			shader->DrawGroup(scene.pSkybox->geometry->groups[i]);
			stats.calls++;
		}
	}

	// default render target
	SetBlending(EBlending::Normal);
	gD3D->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderGBufferDirectionalLights()
{
	CScene &scene = *pCurrentScene;
	CCamera &camera = *pCurrentCamera;

	pDevice->OMSetRenderTargets(1, &gBuffers[3].pView, pGBufferDSView);	
	float clear[4] = { 0, 0, 0, 0 };
	pDevice->ClearRenderTargetView(gBuffers[3].pView, clear);

	CShader *shader = CShaderFactory::GetShader("GBufferDirectionalLight.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	pDevice->IASetInputLayout(shader->pLayout);

	// quad geometry
	float size = 2.0f;
	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, -1.0f+size, 0, 1, 1, 1, 1, 0, 0 },
		{ -1.0f+size, -1.0f+size, 0, 1, 1, 1, 1, tu, 0 },
		{ -1.0f+size, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	pGeometryRenderer->SetNoTexture();	
	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(false);
	SetBlending(EBlending::AdditiveLights);
	SetStencilState(false, false);

	// set g-buffer
	shader->uniforms["colorMap"].SetResource(gBuffers[0].pResView);
	shader->uniforms["normalMap"].SetResource(gBuffers[1].pResView);
	shader->uniforms["depthMap"].SetResource(gBuffers[2].pResView);	
	shader->uniforms["screenScaleRatio"].Set(Vector2(1.0f / tu, 1.0f / tv));

	shader->uniforms["eye"].Set(camera.matrixWorld.GetPosition());	
	
	Matrix4 m = Matrix4::Multiply(camera.matrixProjection, camera.matrixView);	
	m = m.GetInverse();
	shader->uniforms["invertViewProjection"].Set(m);
	shader->uniforms["halfPixel"].Set(Vector2(0.5f / (float)2048, 0.5f / (float)2048));	
	shader->uniforms["projectionMatrix"] = camera.matrixProjection;
	shader->uniforms["viewMatrix"] = camera.matrixView;

	// find directional lights
	for (int i=0; i<scene.lights.Size(); i++)
	{
		CLight *light = scene.lights[i];
		if (light->GetType() != "DirectionalLight") continue;		
	
		// set variables
		shader->uniforms["lightDirection"].Set(light->forward);
		float intensity = light->intensity * 255.0f;
		Clamp(intensity, 0, 255);
		SRGBA color(light->color.r, light->color.g, light->color.b, intensity);
		shader->uniforms["lightColor"].Set(color);			
		
		// shadow
		shader->uniforms["shadow"].Set(light->castShadow);
		if (light->castShadow)
		{
			Matrix4 view = light->GetViewMatrix();				// calculate the view matrix of the light
			Matrix4 viewProjection = Matrix4::Multiply(light->GetProjectionMatrix(), view);	// light projection matrix

			shader->uniforms["shadowMatrix"] = viewProjection;
			shader->uniforms["shadowMap"] = gRenderer.pShadowMapRSView;		
			shader->uniforms["shadowBias"] = 0.003f;
		}

		// add all three buffers 
		pGeometryRenderer->DrawQuad(v);
		pGeometryRenderer->Render(shader);		
	}	

	SetBlending(EBlending::Normal);
	gD3D->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderGBufferPointLights()
{
	CScene &scene = *pCurrentScene;
	CCamera &camera = *pCurrentCamera;

	pDevice->OMSetRenderTargets(1, &gBuffers[3].pView, pGBufferDSView);
		
	CShader *shader = CShaderFactory::GetShader("GBufferPointLight.fx", EShaderAttribute::POSITION);
	pDevice->IASetInputLayout(shader->pLayout);

	// geometry material
	static CMaterial *positionMaterial = new CMaterial();
	positionMaterial->features = 0;

	// sphere and cone geometry
	if (!pSphereGeometry) 
	{
		pSphereGeometry = new CSphereGeometry();
		pSphereGeometry->materials.AddToTail(positionMaterial);
		pSphereGeometry->Initialize();
	}
	if (!pConeGeometry) 
	{
		pConeGeometry = new CCylinderGeometry(0);
		pConeGeometry->materials.AddToTail(positionMaterial);
		pConeGeometry->Initialize();
	}
	

	SetFillMode();	
	SetBlending(EBlending::AdditiveLights);
	SetStencilState(false, false);

	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;

	// set g-buffer
	shader->uniforms["colorMap"].SetResource(gBuffers[0].pResView);
	shader->uniforms["normalMap"].SetResource(gBuffers[1].pResView);
	shader->uniforms["depthMap"].SetResource(gBuffers[2].pResView);	
	shader->uniforms["screenScaleRatio"].Set(Vector2(1.0f / tu, 1.0f / tv));

	// set matrices	
	shader->uniforms["projectionMatrix"] = camera.matrixProjection;
	shader->uniforms["viewMatrix"] = camera.matrixView;
	shader->uniforms["eye"] = camera.matrixWorld.GetPosition();
	shader->uniforms["halfPixel"].Set(Vector2(0.5f / (float)2048, 0.5f / (float)2048));
	

	//camera.matrixView.SetRotationX(45);
	Matrix4 m = Matrix4::Multiply(camera.matrixProjection, camera.matrixView);	
	m = m.GetInverse();
	shader->uniforms["invertViewProjection"].Set(m);
	

	// find point lights
	for (int i=0; i<scene.lights.Size(); i++)
	{
		CLight *light = scene.lights[i];
		if (light->GetType() != "PointLight") continue;	

		// if camera is inside light, turn off culling
		float dist = (light->GetWorldPosition() - camera.GetWorldPosition()).Length();
		gD3D->SetRasterState(0);
		
		// set variables
		light->SetScale(light->range);
		light->UpdateMatrixWorld(true);

		shader->worldMatrix.Set(light->matrixWorld);
		shader->lightPosition.Set(light->GetWorldPosition());
		shader->lightColor.Set(light->color);			
		shader->lightRadius.Set(light->range);
		shader->lightIntensity.Set(light->intensity);
		shader->lightOverbright.Set(light->overbright);

		// render sphere
		shader->DrawGroup(pSphereGeometry->groups[0]);
	}	

	SetBlending(EBlending::Normal);
	gD3D->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderGBufferCombine()
{
	SwapPostRenderTargets();
	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);

	CShader *shader = CShaderFactory::GetShader("GBufferCombine.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	pDevice->IASetInputLayout(shader->pLayout);

	// quad geometry
	float size = 2.0f;
	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, -1.0f+size, 0, 1, 1, 1, 1, 0, 0 },
		{ -1.0f+size, -1.0f+size, 0, 1, 1, 1, 1, tu, 0 },
		{ -1.0f+size, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	pGeometryRenderer->SetNoTexture();	
	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(false);	
	SetStencilState(false, false);

	// set g-buffer
	//shader->uniforms["colorMap"].SetResource(postRenderTargets[1]->pResView);	// draw SSAO
	shader->uniforms["colorMap"].SetResource(gBuffers[0].pResView);
	shader->uniforms["lightMap"].SetResource(gBuffers[3].pResView);
	shader->uniforms["depthMap"].SetResource(gBuffers[2].pResView);
	shader->uniforms["halfPixel"].Set(Vector2(0.5f / (float)2048, 0.5f / (float)2048));
	if (pCurrentScene->fog)
	{
		shader->uniforms["fogColor"].Set(pCurrentScene->fog->color);
		shader->uniforms["fogNear"].Set(pCurrentScene->fog->near);
		shader->uniforms["fogFar"].Set(pCurrentScene->fog->far);
	}

	// set variables
	shader->uniforms["ambientLight"].Set(pCurrentScene->ambientColor);

	// draw full-screen quad
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render(shader);		
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::RenderFinalView()
{
	gD3D->SetDefaultRenderTarget();
	
	// quad geometry
	float size = 2.0f;
	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, -1.0f + size, 0, 1, 1, 1, 1, 0, 0 },
		{ -1.0f + size, -1.0f + size, 0, 1, 1, 1, 1, tu, 0 },
		{ -1.0f + size, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	Clear(false, true);
	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(false);
	SetStencilState(false, false);

	pGeometryRenderer->SetTexture(postRenderTargets[0]->pResView);	// 3rd view substitude with a shadow view
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Reset()
{
	pCurrentCamera = NULL;
	pCurrentScene = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::ReleaseHUDSprites()
{
	hudSprites.PurgeAndDeleteElements();
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::AddSprite( CSprite2D *spr )
{
	hudSprites.Push(spr);
}

CSprite2D* CRenderer::AddSprite( CTexture *tex, float x, float y, SRGBA color /*= WHITE*/, float scaleX /*= 1.0f*/, float scaleY /*= 1.0f*/ )
{
	CSprite2D *spr = new CSprite2D();
	spr->pTexture = tex;
	spr->position.x = x;
	spr->position.y = y;
	spr->color = color;
	spr->scale.x = scaleX;
	spr->scale.y = scaleY;
	hudSprites.Push(spr);
	return spr;
}

CSprite2D* CRenderer::AddSpriteRect( CTexture *tex, float x, float y, float w, float h, SRGBA color /*= WHITE*/ )
{
	CSprite2D *spr = new CSprite2D();
	spr->pTexture = tex;
	spr->position.x = x;
	spr->position.y = y;
	spr->offset = Vector2(-1,-1);
	spr->scale.x = w / tex->width;
	spr->scale.y = h / tex->height;
	spr->color = color;	
	hudSprites.Push(spr);
	return spr;
}

void CRenderer::RemoveSprite( CSprite2D *spr )
{
	hudSprites.Remove(spr);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Addsphere( Vector3 center, float radius, SRGBA color )
{
	if (radius == 0) return;
	int segmentsX = 20;
	int segmentsY = 10;
	float phiStart = 0, phiLength = M_PI*2, thetaStart = 0, thetaLength = M_PI;

	CArray<Vector3> vertices;

	for (int y=0; y<=segmentsY; y++)
	{
		for (int x=0; x<=segmentsX; x++)
		{
			float u = (float)x / segmentsX;
			float v = (float)y / segmentsY;

			float xpos = -radius * cos(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
			float ypos = radius * cos(thetaStart + v * thetaLength);
			float zpos = radius * sin(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);

			vertices.Add( center + Vector3( xpos, ypos, zpos ));
		}
	}

	for (int y=0; y<=segmentsY-1; y++)
	{
		for (int x=1; x<=segmentsX; x++)
		{
			AddLine(vertices[y*(segmentsX+1)+x-1], vertices[y*(segmentsX+1)+x], color);
			AddLine(vertices[y*(segmentsX+1)+x], vertices[(y+1)*(segmentsX+1)+x], color);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Bloom(ID3D10RenderTargetView *pRenderTarget)
{
	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	SetBlending(EBlending::None);
	CShader *shader = CShaderFactory::GetShader("bloom.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	pDevice->IASetInputLayout(shader->pLayout);

	// quad geometry
	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, 1.0f, 0, 1, 1, 1, 1, 0, 0 },
		{ 1.0f, 1.0f, 0, 1, 1, 1, 1, tu, 0 },
		{ 1.0f, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(true);	
	SetStencilState(false, false);

	// set g-buffer
	shader->uniforms["diffuseTexture"].SetResource(postRenderTargets[1]->pResView);
		
	// draw full-screen quad
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render(shader);		
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::SSAO()
{
	// Render SSAO into the color buffer
	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	SetBlending(EBlending::None);

	CShader *shader = CShaderFactory::GetShader("ssao.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	CTexture *pNoise = CTexture::Get("textures/ssao.png");
	pDevice->IASetInputLayout(shader->pLayout);

	// quad geometry
	float tu = (float)gEngine.width / 2048;
	float tv = (float)gEngine.height / 2048;	
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, tv },
		{ -1, 1.0f, 0, 1, 1, 1, 1, 0, 0 },
		{ 1.0f, 1.0f, 0, 1, 1, 1, 1, tu, 0 },
		{ 1.0f, -1, 0, 1, 1, 1, 1, tu, tv }
	};

	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(true);	
	SetStencilState(false, false);

	// bind g-buffer
	shader->uniforms["normalMap"].SetResource(gBuffers[EGBuffer::NormalSpecular].pResView);
	shader->uniforms["depthMap"].SetResource(gBuffers[EGBuffer::Depth].pResView);
	shader->uniforms["rnm"].SetResource(pNoise->GetTexture());

	CCamera &camera = *pCurrentCamera;
	shader->uniforms["near"].Set(camera.near);
	shader->uniforms["far"].Set(camera.far);
	shader->uniforms["viewMatrix"] = camera.matrixView;

	Matrix4 m = Matrix4::Multiply(camera.matrixProjection, camera.matrixView);	
	m = m.GetInverse();
	shader->uniforms["invertViewProjection"].Set(m);

	// draw full-screen quad
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render(shader);		

	// blur
	SwapPostRenderTargets();
	pDevice->OMSetRenderTargets(1, &postRenderTargets[0]->pView, pGBufferDSView);
	shader = CShaderFactory::GetShader("blur.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);
	
	shader->uniforms["colorMap"].SetResource(postRenderTargets[1]->pResView);
	Vector2 texelSize(1.0f/2048, 1.0f/2048);
	shader->uniforms["texelSize"].Set(texelSize);

	// draw full-screen quad
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render(shader);		

	// default render target
	SetBlending(EBlending::Normal);
	gD3D->SetDefaultRenderTarget();
}


//////////////////////////////////////////////////////////////////////////
void CRenderer::FXAA(ID3D10RenderTargetView *pRenderTarget)
{
	pDevice->OMSetRenderTargets(1, &pRenderTarget, pGBufferDSView);
	SetBlending(EBlending::None);

	CShader *shader = CShaderFactory::GetShader("fxaa.fx", EShaderAttribute::COLOR | EShaderAttribute::POSITION | EShaderAttribute::TEXCOORD);	
	pDevice->IASetInputLayout(shader->pLayout);

	// quad geometry
	SVertexColorTextured v[4] = {
		{ -1, -1, 0, 1, 1, 1, 1, 0, 1 },
		{ -1, 1.0f, 0, 1, 1, 1, 1, 0, 0 },
		{ 1.0f, 1.0f, 0, 1, 1, 1, 1, 1, 0 },
		{ 1.0f, -1, 0, 1, 1, 1, 1, 1, 1 }
	};

	SetFillMode();
	gD3D->SetRasterState(0);
	pGeometryRenderer->SetUseAlpha(true);	
	SetStencilState(false, false);

	// bind g-buffer
	shader->uniforms["TheTexture"].SetResource(offBuffer.pResView);
	shader->uniforms["World"].Set(pCurrentCamera->matrixWorld);
	shader->uniforms["View"].Set(pCurrentCamera->matrixView);
	shader->uniforms["Projection"].Set(pCurrentCamera->matrixProjection);

	float N = 0.40f;
	Vector2 texelSize(1.0f/2048, 1.0f/2048);
	shader->uniforms["InverseViewportSize"].Set(texelSize);
	shader->uniforms["SubPixelAliasingRemoval"].Set(0.75f);
	shader->uniforms["EdgeThreshold"].Set(0.166f);
	shader->uniforms["EdgeThresholdMin"].Set(0.0f);
	shader->uniforms["ConsoleEdgeSharpness"].Set(8.0f);
	shader->uniforms["ConsoleEdgeThreshold"].Set(0.125f);
	shader->uniforms["ConsoleEdgeThresholdMin"].Set(0.0f);
	shader->uniforms["SubPixelAliasingRemoval"].Set(0.75f);
	shader->uniforms["ConsoleSharpness"].Set(Vector4(
		-N / gEngine.width,
		-N / gEngine.height,
		N / gEngine.width,
		N / gEngine.height
		));
	shader->uniforms["ConsoleOpt1"].Set(Vector4(
		-2.0f / gEngine.width,
		-2.0f / gEngine.height,
		2.0f / gEngine.width,
		2.0f / gEngine.height
		));
	shader->uniforms["ConsoleOpt2"].Set(Vector4(
		8.0f / gEngine.width,
		8.0f / gEngine.height,
		-4.0f / gEngine.width,
		-4.0f / gEngine.height
		));
	
	// draw full-screen quad
	pGeometryRenderer->DrawQuad(v);
	pGeometryRenderer->Render(shader);	

	// default render target
	SetBlending(EBlending::Normal);
	gD3D->SetDefaultRenderTarget();
}