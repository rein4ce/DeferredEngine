#pragma once
#include "platform.h"
#include "Utils.h"
#include "Array.h"

class CObject3D;
class CTexture;
class CMaterial;
class CMesh;
class aiNode;

class CModelLoader
{
public:	
	static CObject3D* Load(string filename, CMaterial *material = NULL);
	static CObject3D* Get(string filename);
	static void	Release();
	
private:
	static void ProcessNode(aiNode *node, CObject3D *object);

private:
	static CArray<CTexture*>	textures;
	static CArray<CMaterial*>	materials;
	static CArray<CMesh*>		meshes;

	static map<string, CObject3D*>	modelsMap;		// map of loaded models
};