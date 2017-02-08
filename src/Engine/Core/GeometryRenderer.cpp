#include "platform.h"
#include "GeometryRenderer.h"
#include "Framework.h"
#include "Shader.h"
#include "Engine.h"
#include "VertexFormats.h"
#include "Direct3D.h"


void CGeometryRenderer::Init()
{
	D3D10_BUFFER_DESC desc;
	desc.Usage = D3D10_USAGE_DYNAMIC;
	desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	ID3D10Device *pDevice = Direct3D::Instance()->GetDevice();

	desc.ByteWidth = sizeof(SVertexColorTextured) * 1 * MaxPrimitives;
	DXV( pDevice->CreateBuffer( &desc, NULL, &pPointsBuffer ) );

	desc.ByteWidth = sizeof(SVertexColorTextured) * 2 * MaxPrimitives;
	DXV( pDevice->CreateBuffer( &desc, NULL, &pLinesBuffer ) );

	desc.ByteWidth = sizeof(SVertexColorTextured) * 3 * MaxPrimitives;
	DXV( pDevice->CreateBuffer( &desc, NULL, &pTrianglesBuffer ) );

	points = new SVertexColorTextured[MaxPrimitives];
	lines = new SVertexColorTextured[MaxPrimitives*2];
	triangles = new SVertexColorTextured[MaxPrimitives*3];

	pointsNum =
	linesNum =
	trianglesNum = 0;
	ortho = false;

	pDebugShader = CShaderFactory::GetShader("debug.fx", EShaderAttribute::POSITION | EShaderAttribute::COLOR | EShaderAttribute::TEXCOORD);
}

void CGeometryRenderer::Release()
{
	SAFE_RELEASE(pPointsBuffer);
	SAFE_RELEASE(pLinesBuffer);
	SAFE_RELEASE(pTrianglesBuffer);
	SAFE_DELETE_ARRAY(points);
	SAFE_DELETE_ARRAY(lines);
	SAFE_DELETE_ARRAY(triangles);
}

void CGeometryRenderer::Render(CShader *pShader)
{
	ID3D10Device *pDevice = Direct3D::Instance()->GetDevice();
	
	if (pShader == NULL) pShader = pDebugShader;

	int vertexSize = sizeof(SVertexColorTextured);

	uint32 stride = vertexSize;
	uint32 offset = 0;
	pDevice->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);

	// Copy the buffers
	SVertexColor *pData = NULL;
	

	if ( SUCCEEDED( pPointsBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, points, vertexSize * pointsNum );
		pPointsBuffer->Unmap();
	}

	HRESULT result;
	if ( SUCCEEDED( result = pLinesBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, lines, vertexSize * linesNum * 2 );
		pLinesBuffer->Unmap();
	}

	if ( SUCCEEDED( pTrianglesBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>( &pData ))))
	{
		memcpy( pData, triangles, vertexSize * trianglesNum * 3 );
		pTrianglesBuffer->Unmap();
	}

	// Points
	if (pointsNum > 0)
	{		
		pDevice->IASetVertexBuffers(0, 1, &pPointsBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
		pShader->Draw(pointsNum);
		pointsNum = 0;
	}	

	// Lines
	if (linesNum > 0)
	{
		pDevice->IASetVertexBuffers(0, 1, &pLinesBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		pShader->Draw(linesNum*2);
		linesNum = 0;
	}

	// Triangles
	if (trianglesNum > 0)
	{
		pDevice->IASetVertexBuffers(0, 1, &pTrianglesBuffer, &stride, &offset);	
		pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pShader->Draw(trianglesNum*3);
		trianglesNum = 0;
	}

	ortho = false;
}

//////////////////////////////////////////////////////////////////////////
void CGeometryRenderer::DrawLine( Vector3 &a, Vector3 &b, SRGBA color /*= WHITE*/ )
{
	if (linesNum == MaxPrimitives) return;
	if (ortho)
	{
		ConvertToOrtho(a.x, a.y);
		ConvertToOrtho(b.x, b.y);
	}
	SVertexColorTextured A = { a.x, a.y, a.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	SVertexColorTextured B = { b.x, b.y, b.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	lines[linesNum*2+0] = A;
	lines[linesNum*2+1] = B;
	linesNum++;
}

void CGeometryRenderer::DrawTriangle( Vector3 &a, Vector3 &b, Vector3 &c, SRGBA color /*= WHITE*/ )
{
	if (trianglesNum == MaxPrimitives) return;
	if (ortho)
	{
		ConvertToOrtho(a.x, a.y);
		ConvertToOrtho(b.x, b.y);
		ConvertToOrtho(c.x, c.y);
	}
	SVertexColorTextured A = { a.x, a.y, a.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	SVertexColorTextured B = { b.x, b.y, b.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	SVertexColorTextured C = { c.x, c.y, c.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	triangles[trianglesNum*3+0] = A;
	triangles[trianglesNum*3+1] = B;
	triangles[trianglesNum*3+2] = C;
	trianglesNum++;
}

void CGeometryRenderer::DrawTriangle( SVertexColorTextured v[3] )
{
	if (trianglesNum == MaxPrimitives) return;
	if (ortho)
		for (int i=0; i<3; i++) ConvertToOrtho(v[i].x, v[i].y);
	triangles[trianglesNum*3+0] = v[0];
	triangles[trianglesNum*3+1] = v[1];
	triangles[trianglesNum*3+2] = v[2];
	trianglesNum++;
}

void CGeometryRenderer::DrawQuad( Vector3 &a, Vector3 &b, SRGBA color /*= WHITE*/ )
{
	if (trianglesNum >= MaxPrimitives-1) return;
	if (ortho)
	{
		ConvertToOrtho(a.x, a.y);
		ConvertToOrtho(b.x, b.y);	
	}
	SVertexColorTextured A = { a.x, a.y, a.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 0 };
	SVertexColorTextured B = { b.x, a.y, a.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 1, 0 };
	SVertexColorTextured C = { b.x, b.y, b.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 1, 1 };
	SVertexColorTextured D = { a.x, b.y, b.z, (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255, 0, 1 };
	triangles[trianglesNum*3+0] = A;
	triangles[trianglesNum*3+1] = B;
	triangles[trianglesNum*3+2] = C;
	trianglesNum++;
	triangles[trianglesNum*3+0] = A;
	triangles[trianglesNum*3+1] = C;
	triangles[trianglesNum*3+2] = D;
	trianglesNum++;
}

void CGeometryRenderer::DrawQuad( SVertexColorTextured v[4] )
{
	if (trianglesNum >= MaxPrimitives-1) return;
	if (ortho)
		for (int i=0; i<4; i++) ConvertToOrtho(v[i].x, v[i].y);
	triangles[trianglesNum*3+0] = v[0];
	triangles[trianglesNum*3+1] = v[1];
	triangles[trianglesNum*3+2] = v[2];
	trianglesNum++;
	triangles[trianglesNum*3+0] = v[0];
	triangles[trianglesNum*3+1] = v[2];
	triangles[trianglesNum*3+2] = v[3];
	trianglesNum++;
}

void CGeometryRenderer::SetTexture( CTexture *tex )
{
	SetTexture(tex->GetTexture());	
}

void CGeometryRenderer::SetTexture( ID3D10ShaderResourceView *tex )
{
	if (!tex)
	{		
		pDebugShader->uniforms["useTexture"].Set(false);
	}
	else
	{
		pDebugShader->uniforms["diffuseTexture"].SetResource(tex);
		pDebugShader->uniforms["useTexture"].Set(true);
	}	
}

void CGeometryRenderer::SetUseAlpha( bool use )
{
	pDebugShader->uniforms["useAlpha"] = use;
}

void CGeometryRenderer::SetNoTexture()
{
	pDebugShader->uniforms["useTexture"].Set(false);
}

void CGeometryRenderer::Reset()
{
	trianglesNum = 0;
	linesNum = 0;
	pointsNum = 0;
}

void CGeometryRenderer::ConvertToOrtho(float &x, float &y)
{
	x /= gEngine.width;		// 0-1
	y /= gEngine.height;
	x *= 2;						// 0-2
	y *= 2;
	x -= 1;
	y -= 1;					// -1-1
	y *= -1;
}