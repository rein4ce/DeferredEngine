#include "platform.h"
#include "Terrain.h"
#include "Resources.h"
#include "Geometries.h"
#include "Mesh.h"
#include "Material.h"
#include "CVar.h"
#include "Game.h"
#include "Renderer.h"
#include "Collision.h"
#include "..\NewtonWin-2.30\sdk\Newton.h"
#include "Utils3D.h"

const float CTerrain::TileSize = 0.5f; //1.0f;

CTerrain gTerrain;



//////////////////////////////////////////////////////////////////////////
CTerrainChunk::CTerrainChunk()
{
	pGeometry = NULL;	
	pBody = NULL;
}

CTerrainChunk::~CTerrainChunk()
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// Create geometry for this group
//////////////////////////////////////////////////////////////////////////
void CTerrainChunk::Create(int x, int y)
{
	Release();
	this->x = x;
	this->y = y;

	// create cutout of heightfield
	float *heightmap = new float[(CTerrain::ChunkSize+1) * (CTerrain::ChunkSize+1)];
	for (int j=0; j<CTerrain::ChunkSize+1; j++)
		for (int i=0; i<CTerrain::ChunkSize+1; i++)
		{
			float h = 0.0f;
			if (x+i != gTerrain.width-1 && y+j != gTerrain.height-1)
				h = (float)gTerrain.GetHeight(x * CTerrain::ChunkSize+i, y * CTerrain::ChunkSize+j);
			heightmap[j * (CTerrain::ChunkSize+1) + i] = h;
		}

	// create geometry
	pGeometry = new CHeightmapGeometry(
			CTerrain::TileSize * CTerrain::ChunkSize,
			CTerrain::TileSize * CTerrain::ChunkSize,
			CTerrain::ChunkSize,		// number of tiles is ChunkSize, number of points needed ChunkSize+1
			CTerrain::ChunkSize,
			heightmap );		

	// add material
	static CMaterial *mat = new CMaterial(WHITE);
	mat->pTexture =  gTerrain.pShadowTexture;//CTexture::Get("textures/ground.jpg");
	mat->features = EShaderFeature::LIGHT | EShaderFeature::FOG | EShaderFeature::SHADOW | EShaderFeature::TEXTURE;
	mat->specularPower = 0.6f;
	mat->specular = 0.6f;
	mat->receiveShadow = false;
	mat->useAlphaSpecular = true;
	pGeometry->materials.AddToTail(mat);

	// TEMP: change UV
	for (int i=0; i<pGeometry->faces.Size(); i++)
		for (int v=0; v<3; v++)
		{	
			pGeometry->faces[i].texcoord[v].x /= (CTerrain::ChunkSize/8.0f);
			pGeometry->faces[i].texcoord[v].y /= (CTerrain::ChunkSize/8.0f);
			pGeometry->faces[i].texcoord[v].x += (float)x * 1.0f/(CTerrain::ChunkSize/8.0f);
			pGeometry->faces[i].texcoord[v].y += (float)y * 1.0f/(CTerrain::ChunkSize/8.0f);			
		}

	pGeometry->Initialize();	

	delete [] heightmap;
}

void CTerrainChunk::Release()
{
	SAFE_DELETE(pGeometry);
}	

//////////////////////////////////////////////////////////////////////////
void CTerrainChunk::CreateCollision()
{
	static int counter = 0;
	float start = GetFloatTime();
	NewtonCollision *col = NewtonCreateTreeCollision(pGame->pWorld, 0);
	NewtonTreeCollisionBeginBuild(col);

	// create collision mesh
	for (int i=0; i<pGeometry->faces.Size(); i++)
	{
		Face3 &face = pGeometry->faces[i];
		Vector3 v[] = { 
			pGeometry->vertices[face.a], 
			pGeometry->vertices[face.b], 
			pGeometry->vertices[face.c]
		};
		NewtonTreeCollisionAddFace(col, 3, &v[0][0], sizeof(Vector3), 1);
	}
	NewtonTreeCollisionEndBuild(col, 0);

	// create body		
	float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, x*CTerrain::ChunkSize,0,y*CTerrain::ChunkSize,1 };

	pBody = NewtonCreateBody(pGame->pWorld, col, &m[0]);
	NewtonBodySetCollision(pBody, col);	
	NewtonReleaseCollision(pGame->pWorld, col);
	float end = GetFloatTime();
}


//////////////////////////////////////////////////////////////////////////
CTerrain::CTerrain()
{
	width = height = 0;
	chunksX = chunksY = 0;
	pHeightmap = NULL;
	pHeightmapTexture = NULL;
	pMesh = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Load heightmap file and generate terrain from it
//////////////////////////////////////////////////////////////////////////
bool CTerrain::Load( string filename )
{
	CImage *pImage = new CImage(filename);
	if (!pImage->loaded) return false;

	pHeightmapTexture = new CTexture(pImage);
	
	// create arrays
	Create(pImage->width, pImage->height);

	// copy heightmap data
	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			float h = (float)pImage->GetPixel(x, y).a;
			h = h * ((h+100) / 255) *0.65f;// / 255.0f * 255.0f;//  / 25.0f;
			if (h>2) h += frand(-0.1f, +0.1f);
			pHeightmap[y * width + x] = h;
		}

	// create shadow texture
	CreateShadowTexture(Vector3(1,-1,1));

	// create chunks
	for (int x=0; x<chunksX; x++)
		for (int y=0; y<chunksY; y++)
		{
			CTerrainChunk *chunk = &pChunks[y * chunksX + x];
			chunk->Create(x, y);
		}

	// create final mesh		
	pMesh = new CObject3D();
	for (int i=0; i<gTerrain.chunksX * gTerrain.chunksY; i++)
	{
		CMesh *mesh = new CMesh(gTerrain.pChunks[i].pGeometry);
		pMesh->Add(mesh);
		float size = (CTerrain::ChunkSize) * CTerrain::TileSize ;
		mesh->SetPosition(gTerrain.pChunks[i].x * size, 0, gTerrain.pChunks[i].y * size);
		mesh->name = "Terrain Chunk";
		
		SBBox bbox = gTerrain.pChunks[i].pGeometry->boundingBox;
		Vector3 bboxSize = bbox.GetSize();
		CMesh *box = new CMesh( new CCubeGeometry(bboxSize.x, bboxSize.y, bboxSize.z) );
		box->Move(bboxSize.x/2 + bbox.min.x, bboxSize.y/2 + bbox.min.y, bboxSize.z/2+bbox.min.z);
		box->color = SRGBA(rand()%100+155, rand()%100+155, rand()%100+155, 255);
	}

	// create quad tree
	STerrainQuadTreeNode quad = { SBBox(), 0, 0, width, height };	
	pQuadTree = new CQuadTree<STerrainQuadTreeNode>(quad);
	CreateQuadTree(pQuadTree, log2(width)-2);

	// center the terrain around 0,0,0
	pMesh->SetPosition(-gTerrain.width * CTerrain::TileSize/2,-0.05f,-gTerrain.height * CTerrain::TileSize/2);
	
	// create collision
	CreateCollision();

	SAFE_DELETE(pImage);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrain::CreateQuadTree(CQuadTree<STerrainQuadTreeNode> *node, int stepsLeft)
{			
	float min, max;
	GetMinMaxHeight(node->data.x, node->data.y, node->data.width, node->data.height, OUT min, OUT max);
	node->data.bbox.min = Vector3(node->data.x, min, node->data.y);
	node->data.bbox.max = Vector3(node->data.x+node->data.width, max, node->data.y+node->data.height);

	node->data.bbox.min.x *= CTerrain::TileSize;
	node->data.bbox.min.z *= CTerrain::TileSize;
	node->data.bbox.max.x *= CTerrain::TileSize;
	node->data.bbox.max.z *= CTerrain::TileSize;

	node->data.bbox.min.x -= gTerrain.width * CTerrain::TileSize/2;
	node->data.bbox.max.x -= gTerrain.width * CTerrain::TileSize/2;
	node->data.bbox.min.z -= gTerrain.height * CTerrain::TileSize/2;
	node->data.bbox.max.z -= gTerrain.height * CTerrain::TileSize/2;

	if (stepsLeft == 0) return;
	
	node->hasChildren = true;
	STerrainQuadTreeNode quads[4] = 
	{
		{ SBBox(), node->data.x, node->data.y, node->data.width/2, node->data.height/2 },
		{ SBBox(), node->data.x+node->data.width/2, node->data.y, node->data.width/2, node->data.height/2 },
		{ SBBox(), node->data.x+node->data.width/2, node->data.y+node->data.height/2, node->data.width/2, node->data.height/2 },
		{ SBBox(), node->data.x, node->data.y+node->data.height/2, node->data.width/2, node->data.height/2 }		
	};
	
	for (int i=0; i<4; i++)
	{
		node->children[i] = new CQuadTree<STerrainQuadTreeNode>(quads[i]);
		CreateQuadTree(node->children[i], stepsLeft-1);	
	}
}

//////////////////////////////////////////////////////////////////////////
// Get bouding box of the given area
//////////////////////////////////////////////////////////////////////////
void CTerrain::GetMinMaxHeight(int x, int y, int w, int h, OUT float &minHeight, OUT float &maxHeight) 
{
	float _min = 999999.0f, _max = -999999.0f;
	for (int j=y; j<y+h; j++)
		for (int i=x; i<x+w; i++)
		{
			float h = GetHeight(i, j);
			_min = min(_min, h);
			_max = max(_max, h);
		}
	minHeight = _min;
	maxHeight = _max;
}

//////////////////////////////////////////////////////////////////////////
// Create collision
//////////////////////////////////////////////////////////////////////////
void CTerrain::CreateCollision()
{
	USHORT *elevations;
	char *attributes;

	elevations = new USHORT[width * height];
	attributes = new char[width * height];

	memset(attributes, 1, width * height * sizeof(char));

	for (int i=0; i<width*height; i++)
		elevations[i] = (USHORT)(pHeightmap[i] * 100.0f);

	NewtonCollision *collision = NewtonCreateHeightFieldCollision(pGame->pWorld, width, height, 0, elevations, attributes, TileSize, 1.0f/100.0f, 0);
	float m[16] = {1,0,0,0,0,1,0,0,0,0,1,0,pMesh->position.x,pMesh->position.y,pMesh->position.z,1};
	pBody = NewtonCreateBody(pGame->pWorld, collision, m);

	NewtonReleaseCollision(pGame->pWorld, collision);
	delete elevations;
	delete attributes;

	// terrain material
	materialId = NewtonMaterialCreateGroupID(pGame->pWorld);
	NewtonBodySetMaterialGroupID(pBody, materialId);
}

//////////////////////////////////////////////////////////////////////////
// Create heightmap with 0 values
//////////////////////////////////////////////////////////////////////////
void CTerrain::Create( int w, int h )
{
	Release();
	pHeightmap = new float[w * h];

	// create heightmap
	for (int i=0; i<w*h; i++) 
	{
		pHeightmap[i] = 0.0f;
	}

	// create chunks
	chunksX = ceil((float)w / ChunkSize);
	chunksY = ceil((float)h / ChunkSize);
	pChunks = new CTerrainChunk[chunksX * chunksY];

	this->width = w;
	this->height = h;
}

//////////////////////////////////////////////////////////////////////////
// Release all resources
//////////////////////////////////////////////////////////////////////////
void CTerrain::Release()
{
	SAFE_DELETE_ARRAY(pHeightmap);
	SAFE_DELETE_ARRAY(pChunks);
	SAFE_DELETE(pHeightmapTexture);
	SAFE_DELETE(pQuadTree);
	width = height = 0;
	collisionCounter = 0;
}

float CTerrain::GetHeight(int x, int y)
{
	return pHeightmap[y * width + x];
}

//////////////////////////////////////////////////////////////////////////
void CTerrain::CreateShadowTexture(Vector3 sun)
{
	CImage *image = new CImage();
	image->Create(width, height);

	sun *= -1.0f;		// reverse sun vector

	Vector3 vec;
	float shade;

	for (int i=0, j=0, l=width*height*4; i<l; i+=4, j++)
	{
		vec.x = (j-2<0?0:pHeightmap[j-2]) - (j+2>=width*height?0:pHeightmap[j+2]);
		vec.y = 2;
		vec.z = (j-width*2<0?0:pHeightmap[j-width*2]) - (j+width*2>=width*height?0:pHeightmap[j+width*2]);
		vec.Normalize();

		shade = vec.Dot(sun);

		image->pData[i] = max(min((96 + shade * 128.0f) * (0.5f + image->pData[j] * 0.00007f),255.0f),30);
		image->pData[i+1] = max(min((32 + shade * 96.0f) * (0.5f + image->pData[j] * 0.00007f),255.0f),30);
		image->pData[i+2] = max(min((shade * 96.0f) * (0.5f + image->pData[j] * 0.00007f),255.0f),30);
		image->pData[i+3] = max(min(shade * 55.0f, 255),0);
	}

	for (int i=0, l=width*height*4; i<l; i+=4)
	{
		byte v = rand() % 5;
		image->pData[i] = max(min(((int)image->pData[i] + v),255),0);
		image->pData[i+1] = max(min(((int)image->pData[i+1] + v),255),0);
		image->pData[i+2] = max(min(((int)image->pData[i+2] + v),255),0);
	}

	pShadowTexture = new CTexture(image);

	image->Release();
	delete image;
}

//////////////////////////////////////////////////////////////////////////
void CTerrain::Update(float frametime)
{
}

//////////////////////////////////////////////////////////////////////////
void CTerrain::DrawQuadTree(CQuadTree<STerrainQuadTreeNode> *node)
{
	if (!node) return;	
	if (!node->hasChildren)
		gRenderer.AddBBox(node->data.bbox, Vector3(), GREEN);
	if (node->hasChildren)
		for (int i=0; i<4; i++)
			DrawQuadTree(node->children[i]);
}

//////////////////////////////////////////////////////////////////////////
void CTerrain::CreateCollisions()
{
	if (collisionCounter == chunksX * chunksY) return;
	int i=8;
	while (i-->0)
		pChunks[collisionCounter++].CreateCollision();
}

//////////////////////////////////////////////////////////////////////////
bool CTerrain::CastRay( Vector3 &start, Vector3 &end, OUT Vector3 &hitPos )
{
	if (!pQuadTree)
		return false;
	float fraction = 1.0f;
	bool ret = CheckRayAgainstQuadTree(start, end, pQuadTree, hitPos, fraction);
	hitPos = start + (end - start) * fraction;
	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Check quad tree nodes recursively
//////////////////////////////////////////////////////////////////////////
bool CTerrain::CheckRayAgainstQuadTree( Vector3 &start, Vector3 &end, CQuadTree<STerrainQuadTreeNode> *node, OUT Vector3 &hitPos, OUT float &fraction )
{
	// not a leaf yet, proceed to children
	bool found = false;

	// if ray collides with the bbox of this node, go deeper or if final level, start scanning triangles
	if (CCollision::LineBox(start, end, node->data.bbox.min, node->data.bbox.max))
	{		
		if (!node->hasChildren)
		{
			//gRenderer.AddBBox(node->data.bbox, Vector3(), RED);

			// ok, leaf node, check all triangles
			for (int i=node->data.x; i<node->data.x + node->data.width; i++)
				for (int j=node->data.y; j<node->data.y + node->data.height; j++)
				{
					float f = 1.0f;
					bool hit = false;

					Vector3 v[4] = 
					{
						Vector3(i, GetHeight(i,j), j),
						Vector3((i+1), GetHeight(i+1,j), j),
						Vector3((i+1), GetHeight(i+1,j+1), j+1),
						Vector3(i, GetHeight(i,j+1), j+1)
					};

					Vector3 offset = pMesh->position;
					
					for (int k=0; k<4; k++)
					{
						v[k].x *= TileSize;
						v[k].z *= TileSize;
						v[k] += offset; 						
					}

					
					if ((i+j) % 2 != 0)
					{
						Vector3 V[2][3] = { { v[0]+Vector3(0,0.1f,0), v[2]+Vector3(0,0.1f,0), v[3]+Vector3(0,0.1f,0) }, {v[0]+Vector3(0,0.1f,0), v[1]+Vector3(0,0.1f,0), v[2]+Vector3(0,0.1f,0) } };
						hit = CCollision::LineTriangle(v[0], v[2], v[3], start, end, &f);
						if (!hit) hit = CCollision::LineTriangle(v[0], v[1], v[2], start, end, &f);
					}
					else
					{
						Vector3 V[2][3] = { { v[0]+Vector3(0,0.1f,0), v[1]+Vector3(0,0.1f,0), v[3]+Vector3(0,0.1f,0) }, {v[3]+Vector3(0,0.1f,0), v[1]+Vector3(0,0.1f,0), v[2]+Vector3(0,0.1f,0) } };
						hit = CCollision::LineTriangle(v[0], v[1], v[3], start, end, &f);
						if (!hit) hit = CCollision::LineTriangle(v[3], v[1], v[2], start, end, &f);
					}

					if (hit && f < fraction)
					{
						hitPos = start + f * (end - start);
						fraction = f;
						return true;
					}
				}
		}
		else
		{
			for (int i=0; i<4; i++) 
			{
				Vector3 pos;
				float f = 1.0f;
				if (CheckRayAgainstQuadTree(start, end, node->children[i], OUT pos, OUT f))
				{
					found = true;
					if (f < fraction) fraction = f;
				}
			}
		}
	}

	return found;
}