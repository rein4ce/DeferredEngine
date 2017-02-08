#include "platform.h"
#include "Direct3D.h"
#include "Geometry.h"
#include "Utils.h"
#include "Shader.h"
#include "Material.h"
#include "assimp.h"
#include "Renderer.h"
#include "Collision.h"
#include "aiMaterial.h"
#include "aiMesh.h"
#include "aiScene.h"
#include "aiPostProcess.h"
#include "Object3D.h"

uint32 CGeometry::nextId = 0;

//////////////////////////////////////////////////////////////////////////
CGeometryGroup::CGeometryGroup(CGeometry *parent)
{
	pVertexBuffer = NULL;	
	pFaceBuffer = NULL;
	pLineBuffer = NULL;	
	pVertexArray = NULL;

	lineCount = 0;
	vertexCount = 0;
	faceCount = 0;
	vertexSize = 0;
	bufferSize = 0;
	
	pParent = parent;
}

CGeometryGroup::~CGeometryGroup()
{	
	SAFE_RELEASE(pVertexBuffer);	
	SAFE_RELEASE(pFaceBuffer);
	SAFE_RELEASE(pLineBuffer);	
}

//////////////////////////////////////////////////////////////////////////
// Create vertex and index buffer based on faces added by parent geometry
// Must be called after all material shaders have been set and initialized
//////////////////////////////////////////////////////////////////////////
void CGeometryGroup::CreateBuffers()
{
	ID3D10Device *pDevice = gD3D->GetDevice();
	D3D10_BUFFER_DESC desc;
	desc.Usage = pParent->dynamic ? D3D10_USAGE_DYNAMIC : D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;//D3D10_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	vertexCount = faces.Size() * 3;	
	faceCount = faces.Size();

	// create pVertexArray buffer from parent's geometry content
	CreateVertexArray();

	D3D10_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, 0, sizeof(initData));
	initData.pSysMem = pVertexArray;

	// now setup VBO
	desc.ByteWidth = bufferSize = vertexSize * vertexCount * sizeof(float);
	HRESULT hr;
	if ( FAILED( hr = pDevice->CreateBuffer( &desc, &initData, &pVertexBuffer )) )
	{		
		Error("Error creating geometry buffer: %d",hr);
	}

	SAFE_DELETE_ARRAY(pVertexArray);
}

//////////////////////////////////////////////////////////////////////////
// Create vertex array to map the GPU buffer to
//////////////////////////////////////////////////////////////////////////
void CGeometryGroup::CreateVertexArray()
{
	CMaterial *material = pParent->GetMaterial(materialIndex);
	
	// delete previous buffer if present
	SAFE_DELETE_ARRAY(pVertexArray)

	// calculate the size of the vertex data
	uint32 size = 0;
	uint32 attr = material->GetAttributes();

	if (attr & EShaderAttribute::POSITION)		size += 3;
	if (attr & EShaderAttribute::NORMAL)		size += 3;
	if (attr & EShaderAttribute::TEXCOORD)		size += 2;
	//if (attr & EShaderAttribute::TEXCOORD2)		size += 2;
	//if (attr & EShaderAttribute::TANGENT)		size += 3;

	// create the buffer with all faces data
	pVertexArray = new float[size * vertexCount];
	float *p = &pVertexArray[0];
	for (int i=0; i<faces.Size(); i++)
		for (int j=0; j<3; j++)
		{
			int index = faces[i][j];
			if (attr & EShaderAttribute::POSITION) 
			{ 
				*p++ = pParent->vertices[index].x;  
				*p++ = pParent->vertices[index].y; 
				*p++ = pParent->vertices[index].z; 
			}
			if (attr & EShaderAttribute::NORMAL) 
			{ 	
				// Face normals				
				if (!pParent->useVertexNormals)
				{
					*p++ = faces[i].normal.x; 
					*p++ = faces[i].normal.y; 
					*p++ = faces[i].normal.z; 
				}
				else
				{
					// Vertex normals				
					*p++ = faces[i].vertexNormals[j].x; 
					*p++ = faces[i].vertexNormals[j].y; 
					*p++ = faces[i].vertexNormals[j].z; 
				}
				
			}
			/*if (attr & EShaderAttribute::COLOR) 
			{ 
				*p++ = pParent->faces[i].vertexColors[j].r; 
				*p++ = pParent->faces[i].vertexColors[j].g; 
				*p++ = pParent->faces[i].vertexColors[j].b; 
				*p++ = pParent->faces[i].vertexColors[j].a; 
			}*/
			if (attr & EShaderAttribute::TEXCOORD) 
			{ 
				*p++ = faces[i].texcoord[j].x; 
				*p++ = faces[i].texcoord[j].y; 
			}
			/*if (attr & EShaderAttribute::TEXCOORD2) // TODO
			{ 
				*p++ = faces[i].texcoord[j].x;	// TODO: second set of coordinates
				*p++ = faces[i].texcoord[j].y; 
			}
			if (attr & EShaderAttribute::TANGENT) 
			{ 
				*p++ = pParent->faces[i].vertexTangets[j].x; 
				*p++ = pParent->faces[i].vertexTangets[j].y; 
				*p++ = pParent->faces[i].vertexTangets[j].z; 
			}*/
		}

	vertexSize = size;
}

//////////////////////////////////////////////////////////////////////////
// Refresh buffers data
//////////////////////////////////////////////////////////////////////////
void CGeometryGroup::RefreshBuffers()
{
	Assert(pParent->dynamic);	
	float *pData = NULL;
	if (SUCCEEDED(pVertexBuffer->Map(D3D10_MAP_WRITE, 0, reinterpret_cast<void**>(&pData))))
	{
		CreateVertexArray();
		memcpy(pData, pVertexArray, sizeof(float) * min(bufferSize, vertexCount * vertexSize * sizeof(float)));
		pVertexBuffer->Unmap();
		SAFE_DELETE_ARRAY(pVertexArray);		
	}
}

//////////////////////////////////////////////////////////////////////////
CGeometry::CGeometry(string name)
{
	initialized = false;
	id = nextId++;		
	dynamic = false;
	boundingSphere = 0;

	dirtyVertices = true;
	dirtyMorphTargets = true;
	dirtyElements = true;
	dirtyUVs = true;
	dirtyNormals = true;
	dirtyTangents = true;
	dirtyColors = true;

	useVertexNormals = false;

	if (name != "") Load(name);
}

CGeometry::~CGeometry()
{
	//trace("Releasing Geometry");
	Reset();	
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ApplyMatrix(Matrix4 matrix)
{
	Matrix4 matRotation;
	matRotation.ExtractRotation(matrix);

	for (int i=0; i<vertices.Size(); i++)
		matrix.MultiplyVector3(vertices[i]);
	
	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		matRotation.MultiplyVector3(face.normal);
		for (int j=0; j<3; j++)
			matRotation.MultiplyVector3(face.vertexNormals[j]);
		matrix.MultiplyVector3(face.centroid);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeCentroids()
{	
	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		face.centroid.Set(0, 0, 0);

		face.centroid += vertices[face.a];
		face.centroid += vertices[face.b];
		face.centroid += vertices[face.c];

		face.centroid /= 3;		
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeFaceNormals()
{
	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &f = faces[i];

		if (f.a < 0 || f.b < 0 || f.c < 0 || f.a >= vertices.Size() || f.b >= vertices.Size() || f.c >= vertices.Size()) continue;

		Vector3 A = vertices[f.b] - vertices[f.a];
		Vector3 B = vertices[f.c] - vertices[f.a];
		
		f.normal = A.Cross(B).Normalize();		
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeVertexNormals()
{
	CArray<Vector3>	vertices;
	vertices.AddVectorToTail(this->vertices);

	for (int i=0; i<vertices.Size(); i++)
		vertices[i] = Vector3();

	// add up all the face normals for every vertex
	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];		
		vertices[ face.a ] += face.normal;
		vertices[ face.b ] += face.normal;
		vertices[ face.c ] += face.normal;
	}

	// normalize all vertex normals
	for (int i=0; i<vertices.Size(); i++)
		vertices[i].Normalize();

	// copy vertex normals back to the faces
	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		face.vertexNormals[0] = vertices[ face.a ];
		face.vertexNormals[1] = vertices[ face.b ];
		face.vertexNormals[2] = vertices[ face.c ];
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeTangents()
{
	int len = vertices.Size();
	Vector3	*tan1 = new Vector3[len];
	Vector3	*tan2 = new Vector3[len];

	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		
		Vector3 &vA = vertices[face.a];
		Vector3 &vB = vertices[face.b];
		Vector3 &vC = vertices[face.c];

		Vector2 &uvA = face.texcoord[0];
		Vector2 &uvB = face.texcoord[1];
		Vector2 &uvC = face.texcoord[2];

		float x1 = vB.x - vA.x;
		float x2 = vC.x - vA.x;
		float y1 = vB.y - vA.y;
		float y2 = vC.y - vA.y;
		float z1 = vB.z - vA.z;
		float z2 = vC.z - vA.z;

		float s1 = uvB.x - uvA.x;
		float s2 = uvC.x - uvA.x;
		float t1 = uvB.y - uvA.y;
		float t2 = uvC.y - uvA.y;

		float r = 1.0f / ( s1 * t2 - s2 * t1 );
		Vector3 sdir = Vector3( ( t2 * x1 - t1 * x2 ) * r,
								( t2 * y1 - t1 * y2 ) * r,
								( t2 * z1 - t1 * z2 ) * r );
		Vector3 tdir = Vector3( ( s1 * x2 - s2 * x1 ) * r,
								( s1 * y2 - s2 * y1 ) * r,
								( s1 * z2 - s2 * z1 ) * r );

		tan1[face.a] += sdir;
		tan1[face.b] += sdir;
		tan1[face.c] += sdir;

		tan2[face.a] += tdir;
		tan2[face.b] += tdir;
		tan2[face.c] += tdir;
	}

	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		for (int j=0; j<3; j++)
		{
			Vector3 n = face.vertexNormals[j];
			int vertexIndex = face[j];
			Vector3 t = tan1[vertexIndex];
			Vector3 tmp = t;
			tmp -= n * n.Dot(t);
			tmp.Normalize();

			Vector3 tmp2 = face.vertexNormals[j].Cross(t);
			float test = tmp2.Dot( tan2[vertexIndex] );
			float w = test < 0 ? -1 : 1;

			face.vertexTangets[j] = tmp;
		}
	}

	delete tan1;
	delete tan2;
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeBoundingBox()
{
	if (vertices.Size() == 0) return;

	boundingBox.min = vertices[0];
	boundingBox.max = vertices[0];

	for (int i=0; i<vertices.Size(); i++)
	{
		Vector3 &v = vertices[i];
		boundingBox.min.x = min(boundingBox.min.x, v.x);
		boundingBox.min.y = min(boundingBox.min.y, v.y);
		boundingBox.min.z = min(boundingBox.min.z, v.z);

		boundingBox.max.x = max(boundingBox.max.x, v.x);
		boundingBox.max.y = max(boundingBox.max.y, v.y);
		boundingBox.max.z = max(boundingBox.max.z, v.z);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeBoudingSphere()
{
	float radius = 0;
	for (int i=0; i<vertices.Size(); i++)
		radius = max(radius, vertices[i].Length());
	boundingSphere = radius;
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::MergeVertices()
{
	float PRECISION = 1000.0f;//0.0001f;
	CArray<Vector3> unique;
	std::map<string, int> verticesMap;
	std::map<int, int> changes;

	for (int i=0; i<vertices.Size(); i++)
	{
		Vector3 &v = vertices[i];
		stringstream sskey;
		sskey << (int)(v.x * PRECISION) << "_" << (int)(v.y * PRECISION) << "_" << (int)(v.z * PRECISION);
		string key = sskey.str();
		
		if (verticesMap.find(key) == verticesMap.end())
		{
			verticesMap[key] = i;
			unique.AddToTail(vertices[i]);
			changes[i] = unique.Size() - 1;
		}
		else
		{
			changes[i] = changes[ verticesMap[key] ];
		}
	}

	for (int i=0; i<faces.Size(); i++)
	{
		Face3 &face = faces[i];
		face.a = changes[face.a];
		face.b = changes[face.b];
		face.c = changes[face.c];
	}

	vertices.Purge();
	vertices.AddVectorToTail(unique);
}

//////////////////////////////////////////////////////////////////////////
// Initialize geometry for rendering, by creating groups with buffers
// and arrays
//////////////////////////////////////////////////////////////////////////
void CGeometry::Initialize()
{
	if (initialized && !dirtyVertices) return;	
	CreateGroupsByMaterial();

	dirtyVertices = false;
	dirtyMorphTargets = false;
	dirtyElements = false;
	dirtyUVs = false;
	dirtyNormals = false;
	dirtyTangents = false;
	dirtyColors = false;

	initialized = true;
}

//////////////////////////////////////////////////////////////////////////
bool CGeometry::IsDirty()
{
	return dirtyVertices || dirtyMorphTargets || dirtyElements || dirtyUVs || dirtyNormals || dirtyTangents || dirtyColors;
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::CreateGroupsByMaterial()
{
	std::map<int, CGeometryGroup*>	hashGroups;

	groups.PurgeAndDeleteElements();	

	for (int i=0; i<faces.Size(); i++)
	{
		CGeometryGroup *group = NULL;
		Face3 &f = faces[i];
		int material = f.materialIndex;

		// if group for this material hasn't been found, create new one
		std::map<int, CGeometryGroup*>::iterator iter = hashGroups.find(material);
		if (iter == hashGroups.end())
		{
			group = new CGeometryGroup(this);
			group->faces.EnsureCapacity(faces.Size());
			hashGroups[material] = group;
		}
		else
			group = iter->second;

		// set group params and add faces
		group->materialIndex = material;
		group->faces.AddToTail(f);
	}
	
	// now add the created groups
	std::map<int, CGeometryGroup*>::iterator iter = hashGroups.begin();
	while (iter != hashGroups.end()) 
	{
		CGeometryGroup *group = (iter++)->second;
		group->CreateBuffers();		
		groups.AddToTail(group);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGeometry::Reset()
{
	this->vertices.Purge();
	this->colors.Purge();
	this->materials.Purge();
	this->faces.Purge();
	this->groups.PurgeAndDeleteElements();
}

//////////////////////////////////////////////////////////////////////////
bool CGeometry::Load( string filename )
{
	char texturesLoc[20];

	const aiScene *scene = aiImportFile(
		filename.c_str(),
		aiProcess_CalcTangentSpace  |
		aiProcess_Triangulate |
		aiProcess_MakeLeftHanded |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FlipWindingOrder |
		aiProcess_OptimizeMeshes |
		aiProcess_FlipUVs );

	if (!scene)
	{
		trace("Error while loading the model file " + filename);
		return false;
	}

	// get the mesh from the scene
	aiMesh** meshes = scene->mMeshes;					// array of meshes in the file
	aiMaterial** materials = scene->mMaterials;			// array of materials in the file

	aiNode* sceneRoot = scene->mRootNode;
	aiNode** children = sceneRoot->mChildren;
	aiNode* child;

	uint32 *meshID;
	uint32 materialID;

	uint32 numVertices = 0;
	uint32 numTriangles = 0;

	// clean up	
	Reset();

	// buffers for generated geometry
	DWORD *indices;										// indices
	uint32 *attributeBuffer;
	bool rootNode = true;
	int i = 0;
		
	// Process textures
	for (int i=0; i<scene->mNumTextures; i++)
	{
		CMaterial *material = new CMaterial();
		CTexture *texture = new CTexture();		

		trace("MODEL: Loading textures");
		aiTexture *t = scene->mTextures[i];		
		uint32 size = t->mHeight == 0 ? t->mWidth : t->mWidth * t->mHeight * 4;

		texture->FromMemory( "gun", t->pcData, size );
		
		material->pTexture = texture;
		material->features = 
			EShaderFeature::LIGHT |
			EShaderFeature::TEXTURE |
			EShaderFeature::SHADOW;

		this->materials.AddToTail(material);
	}


	int numSubsets = 0;
	int nextVertex = 0;
	int nextIndex = 0;

	CArray<Vector2> texcoords;

	// Process nodes
	while (i < sceneRoot->mNumChildren)
	{
		if (rootNode)
		{
			child = sceneRoot;
			rootNode = false;
		}
		else
		{
			child = children[i];
			i++;
		}

		if (!(child->mNumMeshes > 0)) continue;

		numSubsets += child->mNumMeshes;				// this child of the scene has X meshes
		meshID = child->mMeshes;						// array of meshes indices

		// calculate total amount of vertices/triangles needed
		for (int x = 0; x < child->mNumMeshes; x++)
		{
			numVertices += meshes[meshID[x]]->mNumVertices;
			numTriangles += meshes[meshID[x]]->mNumFaces;
		}

		// copy vertices of the mesh into the array
		for (int x = 0; x < child->mNumMeshes; x++)
		{
			int meshStartIndex = nextVertex;

			//trace(meshes[meshID[x]]->mName.data);

			for (int j = 0; j < meshes[meshID[x]]->mNumVertices; j++)
			{
				Vector3 v = Vector3(
						meshes[meshID[x]]->mVertices[j].x,
						meshes[meshID[x]]->mVertices[j].y,
						meshes[meshID[x]]->mVertices[j].z
					);
				vertices.AddToTail(v);

				texcoords.AddToTail(Vector2(
						meshes[meshID[x]]->mTextureCoords[0][j].x,
						meshes[meshID[x]]->mTextureCoords[0][j].y
					));

				nextVertex++;
			}

			for (int j = 0; j < meshes[meshID[x]]->mNumFaces; j++)
			{
				int n = 0;
				if (x != 0)
					n = j + meshes[meshID[x-1]]->mNumFaces;
				else
					n = j;

				Face3 face = Face3(
						meshes[meshID[x]]->mFaces[j].mIndices[0] + meshStartIndex,
						meshes[meshID[x]]->mFaces[j].mIndices[1] + meshStartIndex,
						meshes[meshID[x]]->mFaces[j].mIndices[2] + meshStartIndex
					);

				face.texcoord[0] = texcoords[face.a];
				face.texcoord[1] = texcoords[face.b];
				face.texcoord[2] = texcoords[face.c];

				face.materialIndex = meshes[meshID[x]]->mMaterialIndex;		// TODO: two materials cause flickering
				
				faces.AddToTail(face);	
			}

		}
	}

	trace("CMesh Loaded " + filename);	
	
	MergeVertices();
	ComputeFaceNormals();
	ComputeVertexNormals();	// artifacts
	ComputeCentroids();

	return true;
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CGeometry::GetMaterial( int index )
{
	if (index == -1 || index >= materials.Size()) 
		return gRenderer.pDefaultMaterial;
	return materials[index];
}

//////////////////////////////////////////////////////////////////////////
// Offset geometry by specified vector
//////////////////////////////////////////////////////////////////////////
void CGeometry::Offset( Vector3 offset )
{
	for (int i=0; i<vertices.Size(); i++)
		vertices[i] += offset;

	dirtyVertices = true;			// rebuild geometry group
	ComputeBoundingShape();
}

//////////////////////////////////////////////////////////////////////////
// Compute bouding box and sphere based on current vertices
//////////////////////////////////////////////////////////////////////////
void CGeometry::ComputeBoundingShape()
{
	ComputeBoundingBox();
	ComputeBoudingSphere();
}

//////////////////////////////////////////////////////////////////////////
CGeometry* CGeometry::Copy()
{
	CGeometry *g = new CGeometry();
	g->vertices.AddVectorToTail(vertices);
	g->colors.AddVectorToTail(colors);
	g->materials.AddVectorToTail(materials);
	g->faces.AddVectorToTail(faces);
	return g;
}