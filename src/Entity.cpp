#include "platform.h"
#include "Entity.h"
#include "Mesh.h"
#include "Tileset.h"
#include "Resources.h"
#include "Material.h"
#include "Game.h"
#include "Entities.h"
#include "CVar.h"
#include "Properties.h"
#include "Serialize.h"
#include "..\NewtonWin-2.30\sdk\Newton.h"


//////////////////////////////////////////////////////////////////////////
CEntity::CEntity()
{
	pMesh = NULL;
	id = -1;
	pTarget = NULL;
	serialized = false;
	maxhp = hp = 100;
	destructible = false;
	pBody = NULL;
	spawned = false;
	editorOnly = false;

	properties.Add(new SPropertyDefine<float>("X", &position.x));
	properties.Add(new SPropertyDefine<float>("Y", &position.y));
	properties.Add(new SPropertyDefine<float>("Z", &position.z));

	properties.Add(new SPropertyDefine<float>("vX", &rotation.x));
	properties.Add(new SPropertyDefine<float>("vY", &rotation.y));
	properties.Add(new SPropertyDefine<float>("vZ", &rotation.z));

	properties.Add(new SPropertyDefine<float>("sX", &scale.x));
	properties.Add(new SPropertyDefine<float>("sY", &scale.y));
	properties.Add(new SPropertyDefine<float>("sZ", &scale.z));

	properties.Add(new SPropertyDefine<string>("Name", &name, PROPERTY_FILE));
}

CEntity::~CEntity()
{
	properties.PurgeAndDeleteElements();
	DeleteBody(); 
}

void CEntity::Update( float frametime )
{
	if (editorOnly)
		this->visible = cv_editor.GetBool();

	CObject3D::Update(frametime);
}

//////////////////////////////////////////////////////////////////////////
// Entity factory function
//////////////////////////////////////////////////////////////////////////
CEntity* CEntity::Create( string name )
{
	if (name == "Car")			return new CCarEntity(); else
	if (name == "SpawnPoint")	return new CSpawnPoint(); else
	if (name == "Turret")		return new CTurret(); else
	if (name == "Generator")	return new CGenerator(); else
	if (name == "Rocket")		return new CRocket(); else
	if (name == "Waypoint")		return new CWaypoint(); else
	if (name == "Enemy")		return new CEnemy(); else
	if (name == "Spawner")		return new CSpawner(); else
	if (name == "Model")		return new CModel(); else
	if (name == "SemiTruck")	return new CSemiTruck(); else
	if (name == "Light")		return new CLightEntity(); else
	if (name == "Particle Emitter")		return new CParticleEmitter(); else
		return new CEntity();
	
	Assert(0, "Unknown entity type");
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DeleteBody()
{
	if (pBody && pGame && pGame->pWorld)
	{
		NewtonDestroyBody(pGame->pWorld, pBody);
		pBody = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Spawn( Vector3 &pos )
{
	SetPosition(pos);
	UpdateMatrixWorld(true);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Save( ofstream &fs )
{
	for (int i=0; i<properties.Size(); i++)
		CSerialize::Write(fs, &properties[i]->GetValue());
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Load( ifstream &fs )
{
	for (int i=0; i<properties.Size(); i++)
	{
		string val;
		CSerialize::Read(fs, &val);
		properties[i]->SetValue(val);
	}
}

//////////////////////////////////////////////////////////////////////////
CBoxEntity::CBoxEntity() : CEntity()
{
	
}

CBoxEntity::~CBoxEntity()
{
	
}



