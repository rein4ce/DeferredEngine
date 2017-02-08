#pragma once
#include "platform.h"
#include "Utils.h"
#include "Utils3D.h"

class CTexture;
class CGeometry;
class CObject3D;
struct Vector3;
class NewtonBody;

//////////////////////////////////////////////////////////////////////////
struct STerrainQuadTreeNode
{
	SBBox		bbox;
	int			x, y, width, height;
};

//////////////////////////////////////////////////////////////////////////
class CTerrainChunk
{
public:
	CTerrainChunk();
	~CTerrainChunk();

	void			Create(int x, int y);
	void			Release();
	void			CreateCollision();

	CGeometry		*pGeometry;				// render geometry
	NewtonBody		*pBody;					// collision body

public:
	int				x, y;
};

//////////////////////////////////////////////////////////////////////////
class CTerrain
{
public:
	static const float	TileSize;
	static const int	ChunkSize = 64;

	CTerrain();

	void		Create(int w, int h);
	bool		Load(string filename);
	void		Release();

	float		GetHeight(int x, int y);
	void		CreateShadowTexture(Vector3 sun);
	void		Update(float realtime);

	void		CreateCollision();
	void		CreateCollisions();
	void		CreateQuadTree(CQuadTree<STerrainQuadTreeNode> *node, int stepsLeft);
	void		GetMinMaxHeight(int x, int y, int w, int h, OUT float &minHeight, OUT float &maxHeight);

	void		DrawQuadTree(CQuadTree<STerrainQuadTreeNode> *node);

	bool		CheckRayAgainstQuadTree(Vector3 &start, Vector3 &end, CQuadTree<STerrainQuadTreeNode> *node, OUT Vector3 &hitPos, OUT float &fraction);
	bool		CastRay(Vector3 &start, Vector3 &end, OUT Vector3 &hitPos );
	
public:
	int			width, height;
	int			chunksX, chunksY;

	int				materialId;

	float			*pHeightmap;
	CTexture		*pHeightmapTexture;
	CTexture		*pShadowTexture;
	CTerrainChunk	*pChunks;

	CObject3D		*pMesh;

	NewtonBody		*pBody;
	int				collisionCounter;				// how many collision meshes have been created

	CQuadTree<STerrainQuadTreeNode>		*pQuadTree;
};

extern CTerrain gTerrain;