#pragma once
#include "platform.h"
#include "Utils.h"
#include "Face.h"
#include "Vector.h"
#include "Array.h"

class CScene;
class CMesh;
class CGeometry;
struct STriangle;
class NewtonWorld;
class NewtonBody;

//////////////////////////////////////////////////////////////////////////
// Types of tile mesh geometry
//////////////////////////////////////////////////////////////////////////
namespace ETileMeshType { enum Type
{
	None			= 0,
	Block			= 1,
	LowerBlock		= 2,
	HigherBlock		= 3,
	Slope			= 4,
	Stair			= 5,
	UpperSlope		= 6,
	Pole			= 7,
	Num
}; };

//////////////////////////////////////////////////////////////////////////
// Types of tile mesh clip faces
//////////////////////////////////////////////////////////////////////////
namespace ETileClipFace { enum Type
{
	None				= 2 << 0,			// doesn't clip anything
	Full				= 2 << 1,
	LowerBlock			= 2 << 2,
	HigherBlock			= 2 << 3,
	SlopeLeftDown		= 2 << 4,
	SlopeRightDown		= 2 << 5,
	SlopeLeftUp			= 2 << 6,
	SlopeRightUp		= 2 << 7,
	StairRight			= 2 << 8,
	StairLeft			= 2 << 9,
	Pole				= 2 << 10	
}; };

//////////////////////////////////////////////////////////////////////////
namespace ETileSide { enum Type
{
	Front,
	Back,
	Left,
	Right,
	Top,
	Bottom
}; };

//////////////////////////////////////////////////////////////////////////
struct STileMeshFace
{
	int						type;		// type of the face depending on rotation of the tile
	CArray<int>				faces;

	STileMeshFace() {}
	STileMeshFace(int type)
	{
		this->type = type;
	}
};

//////////////////////////////////////////////////////////////////////////
// Tile geometry definition
//////////////////////////////////////////////////////////////////////////
struct STileMesh
{
public:
	CMesh*			pMesh;
	CGeometry*		pGeometry;			// complete geometry of the block
	STileMeshFace	faces[6];			// face indexes for each side
	CArray<int>		triangles;			// face indexes for loose triangles
	

	STileMesh();
	STileMesh(CGeometry *geom, int* clipFaces);

	void AssignFaces();

	~STileMesh();
};

//////////////////////////////////////////////////////////////////////////
// Tile type used in the game
//////////////////////////////////////////////////////////////////////////
struct STileType
{
	string					name;				// display name of the tile type
	STileMesh**				pMeshes;			// geometry used for rendering for each rotation
	int						collisionMesh;		// geometry used for collision
	int						textures[6];		// texture indices

	STileType() {}
	void GetCollideTriangles( int x, int y, int z, int rotation, CArray<STriangle> &ret );
};

//////////////////////////////////////////////////////////////////////////
// Instance of the map tile
//////////////////////////////////////////////////////////////////////////
struct SMapTile
{
public:
	static int				TileSideOpposite[6];

public:

	byte					type;
	byte					rotation;
	byte					hp;

	SMapTile()
	{
		type = 0;
		rotation = 0;	
		hp = 255;
	}
	
	SMapTile(byte type, byte rotation = 0)
	{
		this->type = type;
		this->rotation = rotation;
		hp = 255;
	}

public:

	static byte		GetOppositeSide( ETileSide::Type side );
	static byte		GetSideRotated( ETileSide::Type side, int rotation );
	static byte		GetSideRotatedOpposite( ETileSide::Type side, int rotation );
	static Vector3	GetVertexCoordinates(Vector3 &v, int rotation);
	
	STileType*			GetType();			
	bool				IsSideVisible(int x, int y, int z, ETileSide::Type side);
	int					GetTileClipFace( ETileSide::Type side );
	void				GetCollideTriangles(int x, int y, int z, CArray<STriangle>& ret);
};


//////////////////////////////////////////////////////////////////////////
// Chunk of tiles
//////////////////////////////////////////////////////////////////////////
struct SMapChunk
{
	static const int Size = 8;
	
	CMesh*		pMesh;
	int			x, y, z;
	bool		dirty;
	bool		dirtyBody;		
	NewtonBody	*pBody;			// collision body
	
	SMapChunk();
	~SMapChunk();

	void Recreate();
	void RecreateCollision();
};

//////////////////////////////////////////////////////////////////////////
class CLevel 
{
public:
	CLevel(CScene *pScene, NewtonWorld *pWorld);
	virtual ~CLevel();

	void Create(int chunksX, int chunksY, int chunksZ);
	void Reset();

	void				Update();

	SMapTile*			GetTile(int x, int y, int z);
	SMapChunk*			GetChunk(int x, int y, int z);
	STileType*			GetTileType(int x, int y, int z);
	void				UpdateTile(int x, int y, int z);
	void				Recreate();

	void				GetTilesWithinRange(Vector3 pos, float range, OUT int &minX, OUT int &minY, OUT int &minZ, OUT int &maxX, OUT int &maxY, OUT int &maxZ);
	void				GetCollisionFaces(Vector3 pos, float range, CArray<STriangle>& list);

	bool				CastRay(Vector3 start, Vector3 end, OUT Vector3 &pos, OUT SMapTile **tile, OUT Vector3 *position = NULL);
	bool				CastRaySide(Vector3 &start, Vector3 &end, OUT Vector3 &pos, OUT SMapTile **tile, ETileSide::Type &side, OUT Vector3 *position = NULL);
	void				HighlightTile(int x, int y, int z);
	void				UnHighlightTile();

	void				Save(ofstream &fs);
	bool				Load(ifstream &fs);

public:
	int					sizeX, sizeY, sizeZ;
	int					chunksX, chunksY, chunksZ;

	SMapChunk*			chunks;
	SMapTile*			tiles;

	CScene*				pScene;
	NewtonWorld*		pWorld;
	Vector3				offset;				// howmuch is the world offset from 0,0,0
};

extern CLevel		*pLevel;