#pragma once
#include "platform.h"
#include "Utils.h"
#include "Level.h"
#include "Array.h"

class CMaterial;
class CGeometry;

//////////////////////////////////////////////////////////////////////////
class CTileset
{
public:
	static CArray<STileType>			tiles;				// definitions of all tiles in the game
	static CArray<STileMesh**>			meshes;				// tile meshes
	static STileType					emptyTile;			// empty tile
	static CMaterial*					pMaterial;			// tileset
	static CArray<int>					faceClippingHorizontal;	// array of clipping faceType -> faceType | faceType | ...
	static CArray<int>					faceClippingVertical;	// array of clipping faceType -> faceType | faceType | ...

	static void Init();
	static void Release();
	static void GetTileUV( int tile, float tileU, float tileV, float &outU, float &outV );
	static void LoadTileset();
	static int  GetMeshIndex(string name);

	static bool debug;

private:
	static void ParseData(CGeometry *geometry, CArray<float> &v, CArray<float> &vtex);
	static void CreateGeometries();
	static void CreateMeshes();
	static void LoadTypes();	
	static void RasterizeTriangle(Vector2 v[3], int size, OUT bool *grid);
};