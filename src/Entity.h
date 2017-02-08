#pragma once
#include "platform.h"
#include "Utils.h"
#include "Common.h"
#include "Object3D.h"
#include "Utils3D.h"
#include "Array.h"

class CMesh;
class CTexture;
class CMaterial;
class CGame;
class NewtonBody;
struct SProperty;


//////////////////////////////////////////////////////////////////////////
class CEntity : public CObject3D
{
	friend CGame;

	TYPE("Entity");

	CEntity();
	virtual ~CEntity();

public:

	virtual void	Spawn(Vector3 &pos);
	virtual void	Update(float frametime);

	void			DeleteBody();

	static CEntity*		Create(string name);

	virtual void	Save(ofstream &fs);
	virtual void	Load(ifstream &fs);

public:
	int				id;	
	bool			serialized;			// is the entity saved?
	bool			destructible;		// can be destroyed?

	int				maxhp;				// max health
	int				hp;					// health points
	
	CMesh			*pMesh;
	NewtonBody		*pBody;				// physics body (optional)
	SBBox			bbox;
	CObject3D		*pTarget;

	bool			editorOnly;			// visible only inside editor

	CArray<SProperty*>	properties;

private:
	bool			spawned;
};


//////////////////////////////////////////////////////////////////////////
class CBoxEntity : public CEntity
{
public:
	TYPE("Box");

	CBoxEntity();
	virtual ~CBoxEntity();
};

