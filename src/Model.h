#pragma once
#include "entity.h"

class CModel : public CEntity
{
	TYPE("Model");
public:
	CModel(void);
	virtual ~CModel(void);

	virtual void Load(ifstream &fs);
};
