#pragma once
#include "Entity.h"

class CLight;

class CLightEntity : public CEntity
{
	TYPE("Light");
public:
	CLightEntity(void);
	virtual ~CLightEntity(void);

	virtual void Update(float frametime);

public:
	CLight		*pLight;
	SRGBA		color;
};
