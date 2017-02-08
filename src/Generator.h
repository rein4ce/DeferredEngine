#pragma once
#include "entity.h"

class CGenerator : public CEntity
{
	TYPE("Generator");
public:
	CGenerator(void);
	virtual ~CGenerator(void);

	virtual void Spawn(Vector3 &pos);
	virtual void Update(float frametime);

private:
	float		killTime;
	Vector2		falldir;
	Vector3		spawnPos;
	bool		boom;
	int			lastHP;
};
