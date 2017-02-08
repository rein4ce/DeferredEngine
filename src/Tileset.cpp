#include "platform.h"
#include "Tileset.h"
#include "Level.h"
#include "Material.h"
#include "json_spirit.h"
#include "ModelLoader.h"
#include <boost/regex.hpp>
#include "Face.h"
#include "Geometry.h"
#include "Object3D.h"
#include "Mesh.h"

CArray<STileType>	CTileset::tiles;		
CArray<STileMesh**>	CTileset::meshes = NULL;		
STileType			CTileset::emptyTile;	
CMaterial*			CTileset::pMaterial = NULL;
CArray<int>			CTileset::faceClippingHorizontal;
CArray<int>			CTileset::faceClippingVertical;

bool CTileset::debug = false;

//////////////////////////////////////////////////////////////////////////
// Load tile types from the file and create clipping table
//////////////////////////////////////////////////////////////////////////
void CTileset::Init()
{
	// opposite side table
	SMapTile::TileSideOpposite[ETileSide::Front] = ETileSide::Back;
	SMapTile::TileSideOpposite[ETileSide::Back] = ETileSide::Front;
	SMapTile::TileSideOpposite[ETileSide::Left] = ETileSide::Right;
	SMapTile::TileSideOpposite[ETileSide::Right] = ETileSide::Left;
	SMapTile::TileSideOpposite[ETileSide::Top] = ETileSide::Bottom;
	SMapTile::TileSideOpposite[ETileSide::Bottom] = ETileSide::Top;

	// Load texture
	pMaterial = new CMaterial();
	pMaterial->specular = 0.1f;
	pMaterial->pTexture = CTexture::Get("textures/terrain.png");
	pMaterial->useFiltering = true;

	// load tiles from file
	LoadTileset();
	LoadTypes();

	
}

//////////////////////////////////////////////////////////////////////////
void CTileset::Release()
{
	// release meshes along with their geometry
	for (int i=0; i<meshes.Size(); i++)
	{
		for (int j=0; j<4; j++)
			SAFE_DELETE(meshes[i][j]->pGeometry);		
		SAFE_DELETE_ARRAY(meshes[i]);
	}
	
	meshes.Purge();
}


//////////////////////////////////////////////////////////////////////////
// Load tileset geometry from the file and perform analysis
//////////////////////////////////////////////////////////////////////////
void CTileset::LoadTileset()
{
	const string filename = "models/tileset.dae";
	CObject3D *models = CModelLoader::Load(filename);
	int rasterSize = 16;
	CArray<bool*> faceTypes;		// string rasterized faces

	if (!models)
	{
		Error("Error while loading tileset geometry from file %s", filename.c_str());
		return;
	}

	// find all groups and make them into tiles, also reposition their geometry
	float precision = 0.01f;				// resolution we will round vertex positions to
	CArray<CObject3D*> &groups = models->children[0]->children;
	if (debug) Debug("Tileset has %d groups", groups.Size());

	// allocate room for all tiles + empty one
	meshes.EnsureCount(groups.Size() + 1);

	// add first tile - empty
	emptyTile = STileType();
	emptyTile.name = "Empty";
	emptyTile.pMeshes = NULL;
	emptyTile.collisionMesh = ETileMeshType::None;	
	tiles.AddToTail(emptyTile);
	
	int emptyClipFaces[6] = { 0, 0, 0, 0, 0, 0 };
	STileMesh *emptyMesh = new STileMesh(new CGeometry(), emptyClipFaces);
	for (int i=0; i<6; i++)	emptyMesh->faces[i].type = 0;
	STileMesh **emptyMeshArray = new STileMesh*[4];
	for (int i=0; i<4; i++) emptyMeshArray[i] = emptyMesh;
	meshes[0] = emptyMeshArray;

	// add also empty face tile
	bool *emptyFace = new bool[rasterSize * rasterSize];
	memset(emptyFace, 0, rasterSize * rasterSize);
	faceTypes.AddToTail(emptyFace);
	faceClippingHorizontal.AddToTail(0);			// empty face doesn't clip anything
	faceClippingVertical.AddToTail(0);		


	// interate over each group found in the model file
	// and create each of four rotations
	for (int i=0; i<groups.Size(); i++)
	{
		CObject3D *group = groups[i];
		STileMesh **meshesArray = new STileMesh*[4];
		int groupIndex = 0;

		for (int rot=0; rot<4; rot++)
		{			
			STileMesh *mesh = new STileMesh();
			CGeometry *geometry = mesh->pGeometry = group->children[0]->geometry->Copy();

			mesh->pGeometry = geometry;
			meshesArray[rot] = mesh;
			
			if (debug) Debug("Processing group %d",i);		

			// round vertex positions
			for (int j=0; j<geometry->vertices.Size(); j++)
			{
				Vector3 &pos = geometry->vertices[j];
				pos -= group->position;

				// collada export fix
				pos *= 0.0254;				// convert back to meters			
				float temp = -pos.z;			// swap y & z
				pos.z = pos.y;
				pos.y = temp;
				
				pos /= precision;			// make bigger
				pos.x = (int)pos.x;
				pos.y = (int)pos.y;
				pos.z = (int)pos.z;
				pos *= precision;			// scale back
			}

			// reposition to the 0,0,0
			geometry->ComputeBoundingShape();			
			Vector3 min = geometry->boundingBox.min;
			Vector3 max = geometry->boundingBox.max;
	
			float minX = floor(-min.x/2);
			float minZ = floor(-min.z/2);
			groupIndex = minX + 1;
			
			for (int j=0; j<geometry->vertices.Size(); j++)
			{
				Vector3 &pos = geometry->vertices[j];
				pos.x -= floor(geometry->boundingBox.min.x);
				pos.y -= floor(geometry->boundingBox.min.y);
				pos.z -= floor(geometry->boundingBox.min.z);

				// rotate 
				for (int r=0; r<4-rot; r++)
				{
					swap(pos.x, pos.z);
					pos.x = 1.0f - pos.x;
				}
			}
			geometry->ComputeFaceNormals();

			// create UV mapping, depending on the normal of the face
			for (int j=0; j<geometry->faces.Size(); j++)
			{
				Face3 &face = geometry->faces[j];

				// top
				if (face.normal.y > 0.5f)  
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							geometry->vertices[ face[k] ].x,	
							1.0f - geometry->vertices[ face[k] ].z );
				
				// bottom
				if (face.normal.y < -0.5f)
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							geometry->vertices[ face[k] ].x,	
							1.0f - geometry->vertices[ face[k] ].z );

				// left
				if (face.normal.x < -0.5f)
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							1.0f - geometry->vertices[ face[k] ].z,	
							1.0f - geometry->vertices[ face[k] ].y );

				// right
				if (face.normal.x > 0.5f)
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							geometry->vertices[ face[k] ].z,	
							1.0f - geometry->vertices[ face[k] ].y );

				// front
				if (face.normal.z < -0.5f)
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							geometry->vertices[ face[k] ].x,	
							1.0f - geometry->vertices[ face[k] ].y );

				// back
				if (face.normal.z > 0.5f)
					for (int k=0; k<3; k++)				
						face.texcoord[k] = Vector2( 
							1.0f - geometry->vertices[ face[k] ].x,	
							1.0f - geometry->vertices[ face[k] ].y );
			}


			// set general variables
			geometry->useVertexNormals = false;
			geometry->ComputeBoundingShape();		

			// create occlusion maps for each face		
			CArray<Face3*> sides[6];
			
			for (int j=0; j<geometry->faces.Size(); j++)
			{
				Face3 &face = geometry->faces[j];
				Vector3 v[3] = {
					geometry->vertices[ face[0] ],
					geometry->vertices[ face[1] ],
					geometry->vertices[ face[2] ]
				};
				
				// map out all faces to appropriate sides
				if (v[0].x == 0 && v[1].x == 0 && v[2].x == 0) sides[ETileSide::Left].AddToTail(&face); else
				if (v[0].x == 1 && v[1].x == 1 && v[2].x == 1) sides[ETileSide::Right].AddToTail(&face); else
				if (v[0].y == 0 && v[1].y == 0 && v[2].y == 0) sides[ETileSide::Bottom].AddToTail(&face); else
				if (v[0].y == 1 && v[1].y == 1 && v[2].y == 1) sides[ETileSide::Top].AddToTail(&face); else
				if (v[0].z == 0 && v[1].z == 0 && v[2].z == 0) sides[ETileSide::Front].AddToTail(&face); else
				if (v[0].z == 1 && v[1].z == 1 && v[2].z == 1) sides[ETileSide::Back].AddToTail(&face); 
			}

			// now for each side create a rasterized picture
			for (int j=0; j<6; j++)
			{
				string side;
				switch (j)
				{
				case ETileSide::Left: side = "Left"; break;
				case ETileSide::Right: side = "Right"; break;
				case ETileSide::Top: side = "Top"; break;
				case ETileSide::Bottom: side = "Bottom"; break;
				case ETileSide::Front: side = "Front"; break;
				case ETileSide::Back: side = "Back"; break;
				}

				if (debug) Debug("Side %s, triangles count: %d", side.c_str(), sides[j].Size());
				bool *raster = new bool[rasterSize * rasterSize];
				memset(raster, 0, rasterSize * rasterSize);

				
				// map all triangles on this side to the raster grid, use texcoords as triangle vertices
				for (int f=0; f<sides[j].Size(); f++)
				{
					Face3 &face = *(sides[j])[f];
					RasterizeTriangle(face.texcoord, rasterSize, OUT raster);				
				}

				// debug print
				stringstream ss;
				for (int k=0; k<rasterSize*rasterSize; k++)
				{
					if (k % rasterSize == 0) ss << "\\n";
					ss << (raster[k] ? "X" : ".");				
				}
				if (debug) Debug("Rasterized face:\\n%s", ss.str().c_str());

				// find index for the face type (add new face type if not found)
				bool found = false;
				for (int k=0; k<faceTypes.Size(); k++)
				{
					// check if it's the same
					if (0 == memcmp(faceTypes[k], raster, rasterSize * rasterSize))
					{
						// it's the same, k is the face type index											
						mesh->faces[j].type = k;
						if (debug) Debug("Face %s uses type %d", side.c_str(), mesh->faces[j].type);
						found = true;
						delete raster;					// we no longer need this raster
						break;
					}				
				}	

				// if no matching face found, add new one
				if (!found)
				{
					// face type not found, add
					if (debug) Debug("Adding new face type %d", faceTypes.Size());					
					mesh->faces[j].type = faceTypes.Size();
					faceTypes.AddToTail(raster);	// we will delete raster later
				}
				
			}

			// group triangles inside mesh
			mesh->AssignFaces();
			mesh->pMesh = new CMesh(geometry);		// create a mesh for other uses
		}

		// add array of 4 meshes, one for each rotation
		meshes[groupIndex] = meshesArray;
	}


	// for each registered face type perform clipping
	if (debug) Debug("Registered face types: %d", faceTypes.Size());
	for (int i=0; i<faceTypes.Size(); i++)
	{
		if (debug) Debug("Checking clipping for face %d", i);
		faceClippingHorizontal.AddToTail(0);			// we start by not clipping anything
		faceClippingVertical.AddToTail(0);			// we start by not clipping anything

		for (int j=0; j<faceTypes.Size(); j++)
		{
			bool horizDifferent = false;		// is there any difference?
			bool vertDifferent = false;

			// check raster pixels 
			for (int x=0; x<rasterSize; x++)
			{
				if (horizDifferent && vertDifferent) break;
				for (int y=0; y<rasterSize; y++)
				{					
					// horizontal checking - x is flipped
					if (!faceTypes[i][y*rasterSize+x] && faceTypes[j][y*rasterSize+(rasterSize-x-1)]) horizDifferent = true;
					// vertical checking - y is flipped
					if (!faceTypes[i][y*rasterSize+x] && faceTypes[j][(rasterSize-y-1)*rasterSize+x]) vertDifferent = true;
					if (horizDifferent && vertDifferent) break;
				}
			}

			// if there was no difference (main type covers this one) add as clipped
			if (!horizDifferent) faceClippingHorizontal[i] |= (1<<j);
			if (!vertDifferent) faceClippingVertical[i] |= (1<<j);

			if (!horizDifferent) if (debug) Debug("Face type %d clips type %d horizontally", i, j);
			if (!vertDifferent) if (debug) Debug("Face type %d clips type %d vertically", i, j);
		}
		if (debug) Debug("Face %d clipping = %d",i,faceClippingHorizontal[i]);
	}

	// delete rasters
	for (int i=0; i<faceTypes.Size(); i++) SAFE_DELETE_ARRAY(faceTypes[i]);
	faceTypes.Purge();


	

	delete models;
}

//////////////////////////////////////////////////////////////////////////
void CTileset::CreateGeometries()
{

}

//////////////////////////////////////////////////////////////////////////
void CTileset::ParseData( CGeometry *geometry, CArray<float> &v, CArray<float> &vtex )
{

}


struct SJsonTileInfo
{
	string		name;
	string		mesh;
	string		collision;
	vector<int>	textures;
};

const json_spirit::mValue& FindValue(const json_spirit::mObject& obj, const string &name)
{
	json_spirit::mObject::const_iterator i = obj.find(name);
	return i->second;
}

SJsonTileInfo ReadTileInfo(const json_spirit::mObject& obj)
{
	SJsonTileInfo tile;
	tile.name = FindValue(obj,"name").get_str();
	tile.mesh = FindValue(obj,"mesh").get_str();
	tile.collision = FindValue(obj,"collision").get_str();

	const json_spirit::mArray arr = FindValue(obj, "textures").get_array();
	for (vector<int>::size_type i=0; i<arr.size(); i++)
		tile.textures.push_back(arr[i].get_int());

	return tile;
}


//////////////////////////////////////////////////////////////////////////
void CTileset::LoadTypes()
{
	ifstream is( "tileset.txt" );

	// name mesh front right back left top bottom [collision mesh]
	boost::regex re("^(\\w*)\\s+(\\w*)\\s+(\\d*)\\s(\\d*)\\s(\\d*)\\s(\\d*)\\s(\\d*)\\s(\\d*)(\\s+(\\w*))?");
	
	string line;	
	while (std::getline(is, line))
	{
		boost::match_results<std::string::const_iterator> what;
		if (boost::regex_match(line, what, re))
		{
			string name = string(what[1].first, what[1].second);
			string mesh = string(what[2].first, what[2].second);
			int textures[6];
			for (int i=0; i<6; i++) textures[i] = atoi(string(what[3+i].first, what[3+i].second).c_str());
			string collision = string(what[9].first, what[9].second);

			
			if (debug) Debug("Read tile: %s %s", name.c_str(), mesh.c_str());


			// add tile information to the list
			STileType tile;
			tile.name = name;
			tile.pMeshes = meshes[atoi(mesh.c_str())];
			tile.collisionMesh = collision == "" ? atoi(mesh.c_str()) : atoi(collision.c_str());
			tile.textures[ETileSide::Front] = textures[0];
			tile.textures[ETileSide::Right] = textures[1];
			tile.textures[ETileSide::Back] = textures[2];
			tile.textures[ETileSide::Left] = textures[3];
			tile.textures[ETileSide::Top] = textures[4];
			tile.textures[ETileSide::Bottom] = textures[5];

			tiles.AddToTail(tile);
		}
		else
			break;
	}

	if (debug) Debug("Loaded %d tiles from the file", tiles.Size());
}

//////////////////////////////////////////////////////////////////////////
void CTileset::GetTileUV( int tile, float tileU, float tileV, float &outU, float &outV )
{
	int tex_width = 512, tex_height = 512;
	int tileSize = tex_width / 16;
	int x = tile % 16;
	int y = tile / 16;
	tileU = (tileU - 0.5) * 0.99 + 0.5;
	tileV = (tileV - 0.5) * 0.99 + 0.5;
	outU = (float)x + tileU;
	outV = (float)y + tileV;
	outU /= (float)tex_width / tileSize;
	outV /= (float)tex_height / tileSize;
}

//////////////////////////////////////////////////////////////////////////
// Rasterize triangle into a grid of specified size
//////////////////////////////////////////////////////////////////////////
void CTileset::RasterizeTriangle( Vector2 v[3], int size, OUT bool *grid )
{
	// go through all pixels in the grid and check if they are contained within the triangle
	for (int y=0; y<size; y++)
		for (int x=0; x<size; x++)
		{
			float px = ((float)x + 0.5f) / size;
			float py = ((float)y + 0.5f) / size;

			bool inside = IsPointInsideTriangle(Vector2(px, py), v[0], v[1], v[2]);
			if (inside) grid[y * size + x] = true;
		}
}

