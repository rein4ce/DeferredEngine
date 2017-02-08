#pragma once
#include "platform.h"
#include "Entity.h"

class CMesh;
class CTexture;
class CMaterial;

//////////////////////////////////////////////////////////////////////////
class CPlayerEntity : public CEntity
{
public:
	TYPE("Player");

	CPlayerEntity();
	virtual ~CPlayerEntity();

	virtual void Update(float frametime);

public:
	static CTexture			*pTexture;
	static CMaterial		*pMaterial;

	CMesh 			*head ;
	CMesh 			*torso;
	CMesh 			*larm ;
	CMesh 			*rarm ;
	CMesh 			*lleg ;
	CMesh 			*rleg; 
	CMesh 			*helmet;
};