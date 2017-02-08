#include "platform.h"
#include "Model.h"
#include "ModelLoader.h"
#include "Game.h"
#include "TestGame.h"
#include "Geometries\CubeGeometry.h"
#include "Material.h"
#include "Mesh.h"


CModel::CModel(void)
{
	static CObject3D *model = NULL;

	Add(new CMesh(new CCubeGeometry(), new CMaterial(WHITE)));
	CalculateBoundingShape();	

	boundingShape = EBoundingShape::Box;

	serialized = true;
}

CModel::~CModel(void)
{
}

void CModel::Load( ifstream &fs )
{
	CEntity::Load(fs);

	if (name.size() > 0)
	{
		Empty();
		Add(CModelLoader::Get(name));
	}
}
