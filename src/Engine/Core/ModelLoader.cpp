#include "platform.h"
#include "ModelLoader.h"
#include "Mesh.h"
#include "Geometry.h"
#include "Material.h"
#include "Utils.h"
#include "Shader.h"
#include "assimp.h"
#include "aiMaterial.h"
#include "Renderer.h"
#include "aiPostProcess.h"
#include "aiTexture.h"
#include "aiScene.h"

//////////////////////////////////////////////////////////////////////////
CArray<CTexture*>	CModelLoader::textures;
CArray<CMaterial*>	CModelLoader::materials;
CArray<CMesh*>		CModelLoader::meshes;
map<string, CObject3D*>	CModelLoader::modelsMap;

//////////////////////////////////////////////////////////////////////////
CObject3D* CModelLoader::Load( string filename, CMaterial *pMaterial )
{	
	bool debug = false;

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
		Error("Error while loading the model file %s", filename.c_str());
		return NULL;
	}

	// get the mesh from the scene	
	CObject3D* object = new CObject3D();

	textures.Purge();
	materials.Purge();
	meshes.Purge();


	// create textures
	if (debug) trace("Scene has %d textures", scene->mNumTextures);
	for (int i=0; i<scene->mNumTextures; i++)
	{
		aiTexture *t = scene->mTextures[i];
		CTexture *texture = new CTexture();	
		uint32 size = t->mHeight == 0 ? t->mWidth : t->mWidth * t->mHeight * 4;		
		stringstream ss;
		ss << i;
		texture->FromMemory( ss.str(), t->pcData, size );
		textures.AddToTail(texture);
	}

	// create materials
	if (debug) trace("Scene has %d materials", scene->mNumMaterials);
	for (int i=0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial *m = scene->mMaterials[i];

		aiString name;
		if (AI_SUCCESS == m->Get(AI_MATKEY_NAME, name))	
		if (debug) trace("Processing material \"%s\"", name.data);

		CMaterial *material = pMaterial ? pMaterial : new CMaterial();		// use supplies material or create new one

		if (pMaterial) 
		{
			materials.AddToTail(material);
			continue;
		}

		material->features = gRenderer.pDefaultMaterial->features;

		aiColor3D vColor;
		int vInt;
		float vFloat;
		aiString vString;

		if (AI_SUCCESS == m->Get(AI_MATKEY_COLOR_DIFFUSE, vColor)) 
		{
			if (debug) trace("- diffuse: %f, %f, %f", vColor.r, vColor.g, vColor.b );
			material->color = SRGBA( vColor.r * 255.0f, vColor.g * 255.0f, vColor.b * 255.0f, 255 );
		}

		if (AI_SUCCESS == m->Get(AI_MATKEY_COLOR_SPECULAR, vColor)) if (debug) trace("- specular: %f, %f, %f", vColor.r, vColor.g, vColor.b );
		if (AI_SUCCESS == m->Get(AI_MATKEY_COLOR_AMBIENT, vColor)) if (debug) trace("- ambient: %f, %f, %f", vColor.r, vColor.g, vColor.b );
		if (AI_SUCCESS == m->Get(AI_MATKEY_COLOR_EMISSIVE, vColor)) if (debug) trace("- emissive: %f, %f, %f", vColor.r, vColor.g, vColor.b );
		if (AI_SUCCESS == m->Get(AI_MATKEY_COLOR_TRANSPARENT, vColor)) if (debug) trace("- transparent: %f, %f, %f", vColor.r, vColor.g, vColor.b );

		if (AI_SUCCESS == m->Get(AI_MATKEY_ENABLE_WIREFRAME, vInt)) if (debug) trace("- wireframe: %s", vInt == 0 ? "false" : "true" );
		if (AI_SUCCESS == m->Get(AI_MATKEY_TWOSIDED, vInt)) if (debug) trace("- two-sided: %s", vInt == 0 ? "false" : "true" );
		if (AI_SUCCESS == m->Get(AI_MATKEY_BLEND_FUNC, vInt)) if (debug) trace("- blending: %d", vInt );

		if (AI_SUCCESS == m->Get(AI_MATKEY_OPACITY, vFloat)) 
		{
			if (debug) trace("- opacity: %f", vFloat );
			material->opacity = vFloat;
			if (vFloat < 1.0f) material->transparent = true;
		}
		if (AI_SUCCESS == m->Get(AI_MATKEY_SHININESS, vFloat)) if (debug) trace("- shininess: %f", vFloat );
		if (AI_SUCCESS == m->Get(AI_MATKEY_SHININESS_STRENGTH, vFloat)) if (debug) trace("- shininess strength: %f", vFloat );

		for (int j=0; j<m->GetTextureCount(aiTextureType_AMBIENT); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_AMBIENT, j), vString)) if (debug) trace("- texture ambient: %s", vString.data );

		for (int j=0; j<m->GetTextureCount(aiTextureType_DIFFUSE); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, j), vString)) 
			{
				if (debug) trace("- texture diffuse: %s", vString.data );
				string path = vString.data;

				// if it's an embedded texture
				if (path[0] == '*')
				{
					path = path.substr(1);
					int index = atoi(path.c_str());
					if (index >= 0 && index < textures.Size())
					{
						if (debug) trace("Using embedded texture %d", index);
						material->pTexture = textures[index];
						material->features |= EShaderFeature::TEXTURE;
					}
					else
					{
						if (debug) trace("Incorrect embedded texture index %d, textures count: %d", index, textures.Size());
					}
				}
				else
				{
					string dir = GetFileDirectory(filename);
					CTexture *tex = new CTexture(dir + "/" + path);
					material->pTexture = tex;
					material->features |= EShaderFeature::TEXTURE;
				}				
				break;
			}
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_DISPLACEMENT); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_DISPLACEMENT, j), vString)) if (debug) trace("- texture displacement: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_EMISSIVE); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, j), vString)) if (debug) trace("- texture emissive: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_SPECULAR); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, j), vString)) if (debug) trace("- texture specular: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_NORMALS); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, j), vString)) if (debug) trace("- texture normals: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_SHININESS); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_SHININESS, j), vString)) if (debug) trace("- texture shininess: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_OPACITY); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_OPACITY, j), vString)) if (debug) trace("- texture opacity: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_LIGHTMAP); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, j), vString)) if (debug) trace("- texture lightmap: %s", vString.data );
		
		for (int j=0; j<m->GetTextureCount(aiTextureType_REFLECTION); j++)
			if (AI_SUCCESS == m->Get(AI_MATKEY_TEXTURE(aiTextureType_REFLECTION, j), vString)) if (debug) trace("- texture ambient: %s", vString.data );

		materials.AddToTail(material);
	}

	
	// create meshes
	if (debug) trace("Scene has %d meshes", scene->mNumMeshes);
	for (int i=0; i<scene->mNumMeshes; i++)
	{		
		aiMesh *m = scene->mMeshes[i];
		if (!m->HasPositions()) continue;

		CMesh *mesh = new CMesh();
		CGeometry *geom = new CGeometry();

		mesh->name = string(m->mName.data);
		if (mesh->name == "")
		{
			stringstream ss; ss << i; mesh->name = ss.str();
		}

		if (debug) trace("Creating mesh %s", mesh->name.c_str());
		
		// copy the vertices of the mesh
		for (int j=0; j<m->mNumVertices; j++)
		{
			geom->vertices.AddToTail( Vector3( m->mVertices[j].x, m->mVertices[j].y, m->mVertices[j].z ));			
		}

		// copy faces
		for (int j=0; j<m->mNumFaces; j++)
		{
			aiFace &f = m->mFaces[j];
			if (f.mNumIndices != 3) continue;
			Face3 face = Face3( f.mIndices[0], f.mIndices[1], f.mIndices[2] );	
			
			// material index
			face.materialIndex = m->mMaterialIndex;			

			// if mesh contains texcoords, copy them
			if (m->HasTextureCoords(0))
			{
				for (int k=0; k<3; k++)
					face.texcoord[k] = Vector2(
						m->mTextureCoords[0][face.index[k]].x,
						m->mTextureCoords[0][face.index[k]].y
					);
			}

			// if mesh contains tangents, copy those too
			if (m->HasTangentsAndBitangents())
			{
				for (int k=0; k<3; k++)
					face.vertexTangets[k] = Vector3(
						m->mTangents[face.index[k]].x,
						m->mTangents[face.index[k]].y,
						m->mTangents[face.index[k]].z
					);
			}

			// if mesh has normals, copy them
			if (m->HasNormals())
			{
				for (int k=0; k<3; k++)
					face.vertexNormals[k] = Vector3(
						m->mNormals[face.index[k]].x,
						m->mNormals[face.index[k]].y,
						m->mNormals[face.index[k]].z
					);
			}

			// if mesh has normals, copy them
			if (m->HasVertexColors(0))
			{
				for (int k=0; k<3; k++)
					face.vertexColors[k] = SRGBA(
						m->mColors[0][face.index[k]].r,
						m->mColors[0][face.index[k]].g,
						m->mColors[0][face.index[k]].b,
						m->mColors[0][face.index[k]].a
					);
			}

			// check face indices
			face.a = max(0, face.a);
			face.b = max(0, face.b);
			face.c = max(0, face.c);

			geom->faces.AddToTail(face);
		}

		
		// post-process geometry
		geom->ComputeFaceNormals();
		if (!m->HasNormals()) 
		{
			geom->ComputeVertexNormals();
			geom->useVertexNormals = false;
		}
		else
		{
			geom->useVertexNormals = true;
		}

		geom->ComputeCentroids();
		geom->ComputeBoundingShape();

		// geometry should have a complete list of materials, faces use them
		geom->materials = materials;

		// add geometry to model
		mesh->geometry = geom;
		meshes.AddToTail(mesh);
	}


	aiNode *root = scene->mRootNode;

	// add children meshes
	ProcessNode(root, object);

	textures.Purge();
	materials.Purge();
	meshes.Purge();

	return object;
}

//////////////////////////////////////////////////////////////////////////
void CModelLoader::ProcessNode( aiNode *node, CObject3D *object )
{
	for (int i=0; i<node->mNumMeshes; i++)
	{
		object->Add(meshes[ node->mMeshes[i] ]);
	}

	// now process the children hierarchy
	for (int i=0; i<node->mNumChildren; i++)
	{
		CObject3D *child = new CObject3D();
		aiNode *n = node->mChildren[i];

		memcpy(&child->matrixModel, &n->mTransformation, sizeof(n->mTransformation));
		
		child->matrixModel.Decompose( child->position, child->rotation, child->scale );
		child->rotation *= ToDeg;		
				
		ProcessNode(n, child);		
		object->Add(child);
	}
}

//////////////////////////////////////////////////////////////////////////
// Check if a given model file has been already loaded, and if yes, return a copy of it
//////////////////////////////////////////////////////////////////////////
CObject3D* CModelLoader::Get( string filename )
{
	map<string, CObject3D*>::iterator i = modelsMap.find(filename);
	if (i != modelsMap.end())
	{
		return (i->second)->Copy();
	}
	else
	{
		CObject3D *model = Load(filename);
		modelsMap[filename] = model;
		return model->Copy();
	}
}

//////////////////////////////////////////////////////////////////////////
// Released all loaded models
//////////////////////////////////////////////////////////////////////////
void CModelLoader::Release()
{
	trace("CResourceManager Release()");
	for (std::map<string, CObject3D*>::iterator it = modelsMap.begin(); it != modelsMap.end(); it++)
	{
		CObject3D *res = it->second;
		SAFE_DELETE(res);
	}
	modelsMap.clear();		// destructors will be called
}