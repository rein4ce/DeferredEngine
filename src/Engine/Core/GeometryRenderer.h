#pragma once
#include "platform.h"
#include "Utils.h"

struct SVertexColorTextured;
class CShader;
struct Vector3;
struct SRGBA;
class CTexture;

class CGeometryRenderer
{
	static const int MaxPrimitives = 65535;		// max number of primitives
		
public:
	void Init();
	void Release();
	void Render(CShader *pShader = NULL);
	void Reset();

	void DrawLine(Vector3 &a, Vector3 &b, SRGBA color = WHITE);
	void DrawTriangle(Vector3 &a, Vector3 &b, Vector3 &c, SRGBA color = WHITE);
	void DrawTriangle(SVertexColorTextured v[3]);
	void DrawQuad(Vector3 &a, Vector3 &b, SRGBA color = WHITE);
	void DrawQuad(SVertexColorTextured v[4]);

	void SetTexture(CTexture *tex);
	void SetTexture(ID3D10ShaderResourceView *tex);
	void SetNoTexture();
	void SetUseAlpha(bool use);

	void ConvertToOrtho(float &x, float &y);

public:
	bool					ortho;

private:
	ID3D10Buffer			*pTrianglesBuffer;
	ID3D10Buffer			*pLinesBuffer;
	ID3D10Buffer			*pPointsBuffer;

	SVertexColorTextured	*points;
	SVertexColorTextured	*lines;
	SVertexColorTextured	*triangles;

	CShader					*pDebugShader;
	
	int						pointsNum, linesNum, trianglesNum;
};

