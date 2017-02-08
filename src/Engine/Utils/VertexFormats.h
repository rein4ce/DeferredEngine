#pragma once
#include "platform.h"

struct SVertexColor2D
{
	float x, y, z, rhw;
	DWORD color;
};

struct SVertexTextured2D
{
	float x, y, z, rhw;
	float u, v;
};

struct SVertexColorTextured2D
{
	float x, y, z, rhw;
	DWORD color;
	float u, v;
};

struct SVertexColor
{
	float x, y, z;
	float r, g, b, a;
};

struct SVertexTextured
{
	float x, y, z;
	float u, v;
};

struct SVertexTexturedNormal
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

struct SVertexTexturedNormalTangentBinormal
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float tx, ty, tz;
	float bx, by, bz;
};

struct SVertexColorTextured
{
	float x, y, z;
	float r, g, b, a;
	float u, v;
};

#define FVF_XYZCOLOR_2D				(D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define FVF_XYZTEXTURED_2D			(D3DFVF_XYZRHW|D3DFVF_TEX1)
#define FVF_XYZCOLORTEXTURED_2D		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define FVF_XYZCOLOR				(D3DFVF_XYZ|D3DFVF_DIFFUSE)
#define FVF_XYZTEXTURED				(D3DFVF_XYZ|D3DFVF_TEX1)
#define FVF_XYZCOLORTEXTURED		(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)