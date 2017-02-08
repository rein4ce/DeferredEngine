#include "platform.h"
#include "Level.h"
#include "TileGeometry.h"
#include "Scene.h"
#include "Tileset.h"
#include "Primitives.h"
#include "Collision.h"
#include "Material.h"
#include <Newton.h>
#include "Game.h"
#include "ConsolePanel.h"
#include "Face.h"
#include "Vector.h"
#include "Geometry.h"
#include "Mesh.h"
#include "Geometry.h"

CLevel	*pLevel;

int	SMapTile::TileSideOpposite[6];

int gLevelChunksMaterialID;
int gLevelBlocksMaterialID;
int gLevelFloorMaterialID;


//////////////////////////////////////////////////////////////////////////
byte SMapTile::GetOppositeSide( ETileSide::Type side )
{
	return TileSideOpposite[side];
}

//////////////////////////////////////////////////////////////////////////
byte SMapTile::GetSideRotated( ETileSide::Type side, int rotation )
{
	if ( side == ETileSide::Top || side == ETileSide::Bottom ) return side;
	switch (rotation)
	{
	case 0: return side;
	case 3:
		if ( side == ETileSide::Left ) return ETileSide::Back;
		if ( side == ETileSide::Right ) return ETileSide::Front;
		if ( side == ETileSide::Front ) return ETileSide::Left;
		if ( side == ETileSide::Back ) return ETileSide::Right;
		break;
	case 2:
		return GetOppositeSide( side );					
	case 1:
		if ( side == ETileSide::Left ) return ETileSide::Front;
		if ( side == ETileSide::Right ) return ETileSide::Back;
		if ( side == ETileSide::Front ) return ETileSide::Right;
		if ( side == ETileSide::Back ) return ETileSide::Left;
		break;
	}
	return ETileSide::Front;
}

//////////////////////////////////////////////////////////////////////////
Vector3 SMapTile::GetVertexCoordinates( Vector3 &v, int rotation )
{
	float cx = 0, cz = 0;
	switch ( rotation )
	{
	case 0:
		cx = v.x;
		cz = v.z;
		break;

	case 1:
		cx = 1.0f - v.z;
		cz = v.x;
		break;

	case 2:
		cx = 1.0f - v.x;
		cz = 1.0f - v.z;
		break;

	case 3:
		cx = v.z;
		cz = 1.0f - v.x;
		break;
	}
	return Vector3( cx, v.y, cz );
}

//////////////////////////////////////////////////////////////////////////
bool SMapTile::IsSideVisible( int x, int y, int z, ETileSide::Type side )
{
	int clip = GetType()->pMeshes[rotation]->faces[(int)side].type;				// what shape does the currently rendered face have?
	//side = (ETileSide::Type)GetSideRotated( side, rotation );							// now we need the actual direction the face is heading after rotation
	switch ( side )														// determine the position of the neighbour tile
	{
	case ETileSide::Left:		x--; break;
	case ETileSide::Right:		x++; break;
	case ETileSide::Front:		z--; break;
	case ETileSide::Back:		z++; break;
	case ETileSide::Top:		y++; break;
	case ETileSide::Bottom:		y--; break;
	}

	SMapTile *tile = pLevel->GetTile(x,y,z);
	if (!tile) return true;

	if (tile->type == 0) return true;

	byte neighbourSide = GetOppositeSide(side);										// which side of the neighbour's tile we are checking against			
	int neighbourFace = tile->GetTileClipFace((ETileSide::Type)neighbourSide);
	bool clips;
	// depending of whether its horizontal or vertical relationship, choose appropriate clipping array
	switch ( side )														// determine the position of the neighbour tile
	{
	case ETileSide::Left:
	case ETileSide::Right:
	case ETileSide::Front:
	case ETileSide::Back:
		clips = CTileset::faceClippingHorizontal[neighbourFace] & (1 << clip);
		return !clips;

	case ETileSide::Top:
	case ETileSide::Bottom:
		clips = CTileset::faceClippingVertical[neighbourFace] & (1 << clip);
		return !clips;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
int SMapTile::GetTileClipFace( ETileSide::Type side )
{
	if ( type == 0 ) return ETileClipFace::None;
	return GetType()->pMeshes[rotation]->faces[(int)side].type;
}

//////////////////////////////////////////////////////////////////////////
STileType* SMapTile::GetType()
{
	return &CTileset::tiles[type];
}

//////////////////////////////////////////////////////////////////////////
void SMapTile::GetCollideTriangles( int x, int y, int z, CArray<STriangle> &ret )
{
	if (this->type == 0) return;

	STileType *type = GetType();	
	CGeometry *geometry =  type->pMeshes[rotation]->pMesh->geometry;
	Vector3 pos = Vector3(x,y,z);

	for (int f=0; f<6; f++)
		if (IsSideVisible(x,y,z,(ETileSide::Type)f))
		{
			CArray<int> &faceIndexes = type->pMeshes[rotation]->faces[f].faces;			

			for (int i=0; i<faceIndexes.Size(); i++)
			{
				Face3 &face = geometry->faces[ faceIndexes[i] ];
				STriangle tri;

				for (int v=0; v<3; v++)
				{
					tri.v[v] = geometry->vertices[ face.index[v] ] + pos;
				}

				ret.AddToTail(tri);
			}			
		}

		CArray<int> &faceIndexes = type->pMeshes[rotation]->triangles;
		for (int i=0; i<faceIndexes.Size(); i++)
		{
			Face3 &face = geometry->faces[ faceIndexes[i] ];
			STriangle tri;

			for (int v=0; v<3; v++)
			{
				tri.v[v] = geometry->vertices[ face.index[v] ] + pos;
			}

			ret.AddToTail(tri);
		}	

	return;
}

//////////////////////////////////////////////////////////////////////////
void STileType::GetCollideTriangles( int x, int y, int z, int rotation, CArray<STriangle> &ret )
{
	STileType *type = this;	
	CGeometry *geometry =  type->pMeshes[rotation]->pMesh->geometry;
	Vector3 pos = Vector3(x,y,z);

	for (int f=0; f<6; f++)
		{
			CArray<int> &faceIndexes = type->pMeshes[rotation]->faces[f].faces;			

			for (int i=0; i<faceIndexes.Size(); i++)
			{
				Face3 &face = geometry->faces[ faceIndexes[i] ];
				STriangle tri;

				for (int v=0; v<3; v++)
				{
					tri.v[v] = geometry->vertices[ face.index[v] ] + pos;
				}

				ret.AddToTail(tri);
			}			
		}

	CArray<int> &faceIndexes = type->pMeshes[rotation]->triangles;
	for (int i=0; i<faceIndexes.Size(); i++)
	{
		Face3 &face = geometry->faces[ faceIndexes[i] ];
		STriangle tri;

		for (int v=0; v<3; v++)
		{
			tri.v[v] = geometry->vertices[ face.index[v] ] + pos;
		}

		ret.AddToTail(tri);
	}	
}

STileMesh::STileMesh() {};
STileMesh::STileMesh(CGeometry *geom, int* clipFaces)
{
	this->pGeometry = geom;
	this->pMesh = new CMesh(geom);
	AssignFaces();
}

STileMesh::~STileMesh()
{
}

void STileMesh::AssignFaces()
{
	CGeometry *mesh = pGeometry;
	// find triangles for each face
	for (int i = 0; i < mesh->faces.Size(); i++)
	{
		Face3 &f = mesh->faces[i];
		Vector3 &a = mesh->vertices[f.a];
		Vector3 &b = mesh->vertices[f.b];
		Vector3 &c = mesh->vertices[f.c];

		if (a.z == 0 && a.z == b.z && c.z == b.z)	faces[ETileSide::Front].faces.AddToTail(i);		else
			if (a.z == 1 && a.z == b.z && c.z == b.z)	faces[ETileSide::Back].faces.AddToTail(i);		else
				if (a.y == 1 && a.y == b.y && c.y == b.y)	faces[ETileSide::Top].faces.AddToTail(i);		else
					if (a.y == 0 && a.y == b.y && c.y == b.y)	faces[ETileSide::Bottom].faces.AddToTail(i);	else
						if (a.x == 1 && a.x == b.x && c.x == b.x)	faces[ETileSide::Right].faces.AddToTail(i);		else
							if (a.x == 0 && a.x == b.x && c.x == b.x)	faces[ETileSide::Left].faces.AddToTail(i);		else
								triangles.AddToTail(i);
	}
}

SMapChunk::SMapChunk()
{
	pMesh = new CMesh(new CGeometry());
	dirty = true;
	dirtyBody = true;
	pBody = NULL;
};

SMapChunk::~SMapChunk()
{
	SAFE_DELETE(pMesh);
};

//////////////////////////////////////////////////////////////////////////
// Map
//////////////////////////////////////////////////////////////////////////
CLevel::CLevel(CScene *pScene, NewtonWorld *pWorld)
{
	this->pScene = pScene;
	this->pWorld = pWorld;

	tiles = NULL;
	chunks = NULL;

	Reset();
}

CLevel::~CLevel()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CLevel::Create( int sx, int sy, int sz )
{
	Reset();

	this->chunksX = ceil((float)sx / SMapChunk::Size);
	this->chunksY = ceil((float)sy / SMapChunk::Size);
	this->chunksZ = ceil((float)sz / SMapChunk::Size);

	this->sizeX = chunksX * SMapChunk::Size;
	this->sizeY = chunksY * SMapChunk::Size;
	this->sizeZ = chunksZ * SMapChunk::Size;

	offset = Vector3(-sizeX/2, -2, -sizeZ/2);

	tiles = new SMapTile[sizeX * sizeY * sizeZ];
	chunks = new SMapChunk[chunksX * chunksY * chunksZ];

	for (int x=0; x<chunksX; x++)
		for (int y=0; y<chunksY; y++)
			for (int z=0; z<chunksZ; z++)
			{
				SMapChunk* chunk = GetChunk(x,y,z);
				chunk->x = x;
				chunk->y = y;
				chunk->z = z;				
			}
	
	for (int x=0; x<sizeX; x++)
		for (int y=0; y<sizeY; y++)
			for (int z=0; z<sizeZ; z++)
			{
				SMapTile* tile = GetTile(x,y,z);
				tile->type = y == 0 ? 1 : 0;
			}

	

	for (int x=0; x<chunksX; x++)
		for (int y=0; y<chunksY; y++)
			for (int z=0; z<chunksZ; z++)
			{
				SMapChunk* chunk = GetChunk(x,y,z);
				chunk->Recreate();
				pScene->Add(chunk->pMesh);
				chunk->pMesh->SetPosition(offset);
			}
}

//////////////////////////////////////////////////////////////////////////
void CLevel::Reset()
{
	// Remove chunks from the scene
	for (int x=0; x<chunksX; x++)
		for (int y=0; y<chunksY; y++)
			for (int z=0; z<chunksZ; z++)
			{
				SMapChunk* chunk = GetChunk(x,y,z);
				if (chunk->pMesh) pGame->pScene->RemoveObject(chunk->pMesh);
				if (chunk->pBody) NewtonDestroyBody(pGame->pWorld, chunk->pBody);
			}

	sizeX = sizeY = sizeZ = 0;
	chunksX = chunksY = chunksZ = 0;

	SAFE_DELETE_ARRAY(tiles);
	SAFE_DELETE_ARRAY(chunks);
}

//////////////////////////////////////////////////////////////////////////
SMapTile* CLevel::GetTile( int x, int y, int z )
{
	if (x<0 || y<0 || z<0 || x>=sizeX || y>=sizeY || z>=sizeZ) return NULL;
	return &tiles[y*sizeX*sizeZ + z*sizeX + x];
}

SMapChunk* CLevel::GetChunk( int x, int y, int z )
{
	if (x<0 || y<0 || z<0 || x>=chunksX || y>=chunksY || z>=chunksZ) return NULL;
	return &chunks[y*chunksX*chunksZ + z*chunksX + x];
}

//////////////////////////////////////////////////////////////////////////
STileType* CLevel::GetTileType(int x, int y, int z)
{
	SMapTile *tile = GetTile(x,y,z);
	if (!tile) return &CTileset::emptyTile;
	return &CTileset::tiles[tile->type];
}

//////////////////////////////////////////////////////////////////////////
void CLevel::UpdateTile(int x, int y, int z)
{
	if (!GetTile(x,y,z)) return;
	int cx = x / SMapChunk::Size;
	int cy = y / SMapChunk::Size;
	int cz = z / SMapChunk::Size;
	SMapChunk *chunk = GetChunk(cx,cy,cz);

	chunk->dirty = true;
	chunks->dirtyBody = true;

	SMapChunk *c = NULL;

	if (x % SMapChunk::Size == SMapChunk::Size-1) if ((c = GetChunk(cx+1, cy, cz))) c->dirty = true;
	if (x % SMapChunk::Size == 0) if ((c = GetChunk(cx-1, cy, cz))) c->dirty = true;

	if (y % SMapChunk::Size == SMapChunk::Size-1) if ((c = GetChunk(cx, cy+1, cz))) c->dirty = true;
	if (y % SMapChunk::Size == 0) if ((c = GetChunk(cx, cy-1, cz))) c->dirty = true;

	if (z % SMapChunk::Size == SMapChunk::Size-1) if ((c = GetChunk(cx, cy, cz+1))) c->dirty = true;
	if (z % SMapChunk::Size == 0) if ((c = GetChunk(cx, cy, cz-1))) c->dirty = true;

}


//////////////////////////////////////////////////////////////////////////
void SMapChunk::Recreate()
{
	Assert(pMesh && pMesh->geometry);

	pMesh->geometry->vertices.Purge();
	pMesh->geometry->faces.Purge();
	pMesh->geometry->colors.Purge();
	pMesh->geometry->materials.Purge();

	pMesh->geometry->materials.AddToTail(CTileset::pMaterial);
	pMesh->frustumCulled = false;		// TEMP

	int vcounter = 0;

	for (int x = this->x*Size; x < this->x*Size + Size; x++)
		for (int y = this->y*Size; y < this->y*Size + Size; y++)
			for (int z = this->z*Size; z < this->z*Size + Size; z++)
			{
				SMapTile *tile = pLevel->GetTile(x,y,z);
				if (tile->type == 0) continue;

				STileType *type = &CTileset::tiles[tile->type];
				

				// add triangles for the sides
				for (int f=0; f<6; f++)				
					if (tile->IsSideVisible(x,y,z,(ETileSide::Type)f))
					{
						// add traingles for given side
						for (int t=0; t<tile->GetType()->pMeshes[tile->rotation]->faces[f].faces.Size(); t++)
						{
							Vector3 pos = Vector3(x,y,z);
							STileType *type = tile->GetType();
							CGeometry *geom = type->pMeshes[tile->rotation]->pGeometry;
							Face3 face = geom->faces[tile->GetType()->pMeshes[tile->rotation]->faces[f].faces[t]];

							// add vertices for given face
							pMesh->geometry->vertices.AddToTail(geom->vertices[face.a] + pos);
							pMesh->geometry->vertices.AddToTail(geom->vertices[face.b] + pos);
							pMesh->geometry->vertices.AddToTail(geom->vertices[face.c] + pos);
							
							face.a = vcounter++;
							face.b = vcounter++;
							face.c = vcounter++;

							// tile index in the tileset used by this face
							// (determined by the normal of the face)
							int tileIndex = 0;
							if (face.normal.x < -0.5f) tileIndex = type->textures[ETileSide::Left];
							if (face.normal.x >  0.5f) tileIndex = type->textures[ETileSide::Right];
							if (face.normal.y < -0.5f) tileIndex = type->textures[ETileSide::Bottom];
							if (face.normal.y >  0.5f) tileIndex = type->textures[ETileSide::Top];
							if (face.normal.z < -0.5f) tileIndex = type->textures[ETileSide::Front];
							if (face.normal.z >  0.5f) tileIndex = type->textures[ETileSide::Back];

							float u, v;
							for (int i=0; i<3; i++)
							{
								CTileset::GetTileUV( tileIndex, face.texcoord[i].x, face.texcoord[i].y, u, v);
								face.texcoord[i] = Vector2(u,v);
							}							

							pMesh->geometry->faces.AddToTail(face);
						}
					}				

				// add other triangles				
				for (int t=0; t<tile->GetType()->pMeshes[tile->rotation]->triangles.Size(); t++)
				{
					Vector3 pos = Vector3(x,y,z);
					CGeometry *geom = tile->GetType()->pMeshes[tile->rotation]->pGeometry;
					Face3 face = geom->faces[tile->GetType()->pMeshes[tile->rotation]->triangles[t]];

					// add vertices for given face
					pMesh->geometry->vertices.AddToTail(geom->vertices[face.a] + pos);
					pMesh->geometry->vertices.AddToTail(geom->vertices[face.b] + pos);
					pMesh->geometry->vertices.AddToTail(geom->vertices[face.c] + pos);
					
					face.a = vcounter++;
					face.b = vcounter++;
					face.c = vcounter++;

					// tile index in the tileset used by this face
					// (determined by the normal of the face)
					int tileIndex = 0;
					if (face.normal.x < -0.5f) tileIndex = type->textures[ETileSide::Left];
					if (face.normal.x >  0.5f) tileIndex = type->textures[ETileSide::Right];
					if (face.normal.y < -0.5f) tileIndex = type->textures[ETileSide::Bottom];
					if (face.normal.y >  0.5f) tileIndex = type->textures[ETileSide::Top];
					if (face.normal.z < -0.5f) tileIndex = type->textures[ETileSide::Front];
					if (face.normal.z >  0.5f) tileIndex = type->textures[ETileSide::Back];

					float u, v;
					for (int i=0; i<3; i++)
					{
						CTileset::GetTileUV( tileIndex, face.texcoord[i].x, face.texcoord[i].y, u, v);
						face.texcoord[i] = Vector2(u,v);
					}

					pMesh->geometry->faces.AddToTail(face);
				}				
			}
			
	pMesh->geometry->dirtyVertices = true;
	pMesh->geometry->ComputeFaceNormals();
	pMesh->geometry->ComputeBoundingShape();
	pMesh->boundingShape = EBoundingShape::Box;

	dirty = false;
	
	// create also collision mesh for this chunk
	dirtyBody = true;
}

//////////////////////////////////////////////////////////////////////////
void SMapChunk::RecreateCollision()
{
	if (!dirtyBody) return;// || dirty) return;		// wait until chunk is refreshed

	// create collision body
	if (pLevel->pWorld)
	{
		NewtonCollision * col = NewtonCreateTreeCollision(pLevel->pWorld, 0);
		NewtonTreeCollisionBeginBuild(col);

		CArray<Vector3> &verts = pLevel->GetChunk(x,y,z)->pMesh->geometry->vertices;
		for (int i=0; i<pLevel->GetChunk(x,y,z)->pMesh->geometry->faces.Size(); i++)
		{
			Face3 face = pLevel->GetChunk(x,y,z)->pMesh->geometry->faces[i];
			Vector3 v[] = { verts[face.a], verts[face.b], verts[face.c] };

			// offset the vertices
			for (int j=0; j<3; j++) v[j] += pLevel->offset;

			NewtonTreeCollisionAddFace(col, 3, &v[0][0], sizeof(Vector3), 1);
		}
		NewtonTreeCollisionEndBuild(col, 0);

		// create body
		float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		if (pBody)
		{
			NewtonBodySetCollision(pBody, col);			// body exists, replace collision	
		}
		else
		{
			if (pLevel->GetChunk(x,y,z)->pMesh->geometry->faces.Size() > 0)
			{
				pBody = NewtonCreateBody(pLevel->pWorld, col, &m[0]);
				NewtonBodySetMaterialGroupID(pBody, gLevelChunksMaterialID);
			}			
		}		
		NewtonReleaseCollision(pLevel->pWorld, col);
	
	}

	dirtyBody = false;
}

//////////////////////////////////////////////////////////////////////////
void CLevel::GetTilesWithinRange(Vector3 pos, float range, OUT int &minX, OUT int &minY, OUT int &minZ, OUT int &maxX, OUT int &maxY, OUT int &maxZ)
{
	pos += offset;
	Vector3 start = Vector3( pos.x - range, pos.y - range, pos.z - range );
	Vector3 end = Vector3( pos.x + range, pos.y + range, pos.z + range );
	minX = (int)floor( start.x ); maxX = (int)ceil( end.x );
	minY = (int)floor( start.y ); maxY = (int)ceil( end.y );
	minZ = (int)floor( start.z ); maxZ = (int)ceil( end.z );
	Clamp( minX, 0, sizeX );
	Clamp( minY, 0, sizeY );
	Clamp( minZ, 0, sizeZ );
	Clamp( maxX, 0, sizeX );
	Clamp( maxY, 0, sizeY );
	Clamp( maxZ, 0, sizeZ );
}

//////////////////////////////////////////////////////////////////////////
void CLevel::GetCollisionFaces(Vector3 position, float range, CArray<STriangle>& list)
{
	int minX, minY, minZ, maxX, maxY, maxZ;
	GetTilesWithinRange( position, range, OUT minX, OUT minY, OUT minZ, OUT maxX, OUT maxY, OUT maxZ );

	
	for ( int x=minX; x < maxX; x++ )
		for ( int y=minY; y < maxY; y++ )
			for ( int z=minZ; z < maxZ; z++ )
			{
				CArray<STriangle> tris;
				GetTile(x,y,z)->GetCollideTriangles(x,y,z,tris);
				for (int t=0; t<tris.Size(); t++)
					list.AddToTail(tris[t]);
			}
}

//////////////////////////////////////////////////////////////////////////
// Cast ray and check if it hits any tiles, return true if collision
// OUT tilePos		- tile position
// OUT tile			- tile object
// OUT position		- exact collision point (closest one) - default NULL
//////////////////////////////////////////////////////////////////////////
bool CLevel::CastRay( Vector3 start, Vector3 end, OUT Vector3 &tilePos, OUT SMapTile **tile, OUT Vector3 *position )
{
	start -= offset;
	end -= offset;

	// get list of all tiles along the ray
	CArray<Vector3> tiles;
	TraverseGrid3D(start.x,start.y,start.z, end.x,end.y,end.z, 0,0,0, 1, tiles);

	bool found = false;
	float minD = 99990.0f, d = minD;

	// Check the tiles along the ray and find the one we are pointing at
	for (int i=0; i<tiles.Size(); i++)
	{
		Vector3 pos = Vector3(tiles[i].x, tiles[i].y, tiles[i].z);
		SMapTile *T = pLevel->GetTile(tiles[i].x, tiles[i].y, tiles[i].z);
		
		// no tile at this location, we reached the end of the map
		if (T == NULL) continue;
		
		// if empty tile, continue
		if (T->type == 0) continue;

		// get the list of triangles and check if we hit any of them
		CArray<STriangle> tris;
		T->GetCollideTriangles(tiles[i].x, tiles[i].y, tiles[i].z, tris);
		
		for (int t=0; t<tris.Size(); t++)
			if (CCollision::LineTriangle(tris[t].v[0], tris[t].v[1], tris[t].v[2], start, end, &d))
			{
				if (d < minD)
				{
					minD = d;
					tilePos = pos;
					*tile = T;
					found = true;

					// if we want a precise collision point, find closest triangle
					if (position != NULL)
					{
						*position = start + (end - start) * d + offset;
					}
					else
						return true;		// just return collision success
				}
			}

		// if we found closest colliding triangle in this tile, return
		if (found) return true;
	}

	// nope, haven't colided with any of the tiles
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Cast ray checking which side of a tile we are pointing at for
// block placement purposes
//////////////////////////////////////////////////////////////////////////
bool CLevel::CastRaySide( Vector3 &start, Vector3 &end, OUT Vector3 &tilePos, OUT SMapTile **tile, ETileSide::Type &side, OUT Vector3 *position /*= NULL*/ )
{
	start -= offset;
	end -= offset;

	// get list of all tiles along the ray
	CArray<Vector3> tiles;
	TraverseGrid3D(start.x,start.y,start.z, end.x,end.y,end.z, 0,0,0, 1, tiles);

	bool found = false;
	float minD = 99990.0f, d = minD;

	Vector3 faces[6][6] = {
		{	// Front
			Vector3(0,1,0), Vector3(1,1,0), Vector3(1,0,0),
			Vector3(0,1,0), Vector3(1,0,0), Vector3(0,0,0)
		},
		{	// Back
			Vector3(1,1,1), Vector3(0,1,1), Vector3(0,0,1),
			Vector3(1,1,1), Vector3(0,0,1), Vector3(1,0,1)
		},
		{	// Left
			Vector3(0,1,1), Vector3(0,1,0), Vector3(0,0,0),
			Vector3(0,1,1), Vector3(0,0,0), Vector3(0,0,1)
		},
		{	// Right
			Vector3(1,1,0), Vector3(1,1,1), Vector3(1,0,1),
			Vector3(1,1,0), Vector3(1,0,1), Vector3(1,0,0)
		},
		{	// Top
			Vector3(0,1,1), Vector3(1,1,1), Vector3(1,1,0),
			Vector3(0,1,1), Vector3(1,1,0), Vector3(0,1,0)
		},
		{	// Bottom
			Vector3(0,0,0), Vector3(1,0,0), Vector3(1,0,1),
			Vector3(0,0,0), Vector3(1,0,1), Vector3(0,0,1)
		}
	} ;

	// Check the tiles along the ray and find the one we are pointing at
	for (int i=0; i<tiles.Size(); i++)
	{
		Vector3 pos = Vector3(tiles[i].x, tiles[i].y, tiles[i].z);
		SMapTile *T = pLevel->GetTile(tiles[i].x, tiles[i].y, tiles[i].z);

		// no tile at this location, we reached the end of the map
		if (T == NULL) continue;

		// if empty tile, continue
		if (T->type == 0) continue;

		// check each of 6 faces
		for (int t=0; t<6; t++)
		{
			if (CCollision::LineTriangle(faces[t][0] + pos, faces[t][1] + pos, faces[t][2] + pos, start, end, &d) ||
				CCollision::LineTriangle(faces[t][3] + pos, faces[t][4] + pos, faces[t][5] + pos, start, end, &d))
			{
				if (d < minD)
				{
					minD = d;
					tilePos = pos;
					*tile = T;
					found = true;

					if (t == 0) side = ETileSide::Front;
					if (t == 1) side = ETileSide::Back;
					if (t == 2) side = ETileSide::Left;
					if (t == 3) side = ETileSide::Right;
					if (t == 4) side = ETileSide::Top;
					if (t == 5) side = ETileSide::Bottom;

					// if we want a precise collision point, find closest triangle
					if (position != NULL)
					{
						*position = start + (end - start) * d;
					}
					else
						return true;		// just return collision success
				}
			}			
		}

		// if we found closest colliding box
		if (found) return true;
	}

	// nope, haven't colided with any of the tiles
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CLevel::HighlightTile( int x, int y, int z )
{
	if (CTileset::pMaterial->pShader)
		CTileset::pMaterial->pShader->uniforms["highlightedTile"] = Vector3(x,y,z);
}

//////////////////////////////////////////////////////////////////////////
void CLevel::UnHighlightTile()
{
	if (CTileset::pMaterial->pShader)
		CTileset::pMaterial->pShader->uniforms["highlightedTile"] = Vector3(-1,-1,-1);
}

//////////////////////////////////////////////////////////////////////////
void CLevel::Recreate()
{
	for (int i=0; i<chunksX*chunksY*chunksZ; i++)
		chunks[i].dirty = true;
}

//////////////////////////////////////////////////////////////////////////
void CLevel::Save(ofstream &fs)
{
	//ofstream fs(filename.c_str(), ios::out | ios::binary);
	fs.write((char*)&sizeX, sizeof(sizeX));
	fs.write((char*)&sizeY, sizeof(sizeY));
	fs.write((char*)&sizeZ, sizeof(sizeZ));
	
	for (int i=0; i<sizeX*sizeY*sizeZ; i++)
	{
		fs.write((char*)&tiles[i].type, sizeof(tiles[i].type));
		fs.write((char*)&tiles[i].rotation, sizeof(tiles[i].rotation));
	}

	//fs.close();
}

//////////////////////////////////////////////////////////////////////////
bool CLevel::Load(ifstream &fs)
{
	//ifstream fs(filename.c_str(), ios::in | ios::binary);
	if (!fs.is_open()) return false;
	fs.read((char*)&sizeX, sizeof(sizeX));
	fs.read((char*)&sizeY, sizeof(sizeY));
	fs.read((char*)&sizeZ, sizeof(sizeZ));
	Create(sizeX, sizeY, sizeZ);

	for (int i=0; i<sizeX*sizeY*sizeZ; i++)
	{
		fs.read((char*)&tiles[i].type, sizeof(tiles[i].type));
		fs.read((char*)&tiles[i].rotation, sizeof(tiles[i].rotation));
	}

	Recreate();
	//fs.close();
	return true;
}

// Check all chunks if they are dirty and recreate them
void CLevel::Update()
{
	int dirtyCount = 0;
	for (int i=0; i<chunksX * chunksY * chunksZ; i++)
		if (chunks[i].dirty)
		{
			dirtyCount++;
			chunks[i].Recreate();
		}

	if (dirtyCount > 0)
	{
		CONSOLE("%d chunks recreated this frame", dirtyCount);
	}	
}