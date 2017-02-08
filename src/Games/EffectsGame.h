#pragma once
#include "platform.h"
#include "Game.h"
#include <dContainers/dThread.h>
#include <queue>

class CLight;
class CMaterial;
class CMesh;
class CCar;
class NewtonWorld;
class NewtonBody;
class NewtonCollision;
class CSprite3D;
class CCarEntity;
class CSprite3D;
class NewtonUserJoint;

struct SSpawnTile
{
	Vector3 pos;
	Vector3 vel;
};

#define MAX_BULLETS 1024

class CEffectsGame : public CGame
{
public:
	CEffectsGame();
	virtual ~CEffectsGame();
	static CEffectsGame *pInstance;

	void				CreateCar();
	virtual	void		Init();
	virtual void		Reset();
	virtual	void		Update(float frametime, float realtime);
	
	void				UpdatePhysics(float frametime);

	void				RemoveBodies();
	void				ReloadGeometry();

	void				CreateBuilding(float px, float py, float pz, float sizeX, float sizeY, float sizeZ);
	void				CreateVoxelBuilding(int posX, int posY, int posZ, int width, int depth, int storeys, int storyHeight, int tileType = 1);
	void				CreateBoxesScene();
	void				CreateScene();
	void				DoPicking();
	void				FadeOutBoxes();

	void				CreateRocket(Vector3 pos, Vector3 dir);
	void				CreateBullet(Vector3 pos, Vector3 dir);
	void				UpdateBullets(float frametime);
	void				RemoveBullet(int i);

	void				SpawnTile(SSpawnTile &tile);

	static void ApplyForceAndTorqueCallback(const NewtonBody* body, float timestep, int threadIndex);

protected:
	
public:

	CLight			*pLight;
	CMaterial		*mat;
	CArray<CMesh *> sparks;

	CMesh			*pFloor;
	NewtonBody		*pFloorBody;
	NewtonCollision	*pFloorCollision;

	CMesh			*pBox;
	NewtonBody		*pBoxBody;
	NewtonCollision	*pBoxCollision;

	CArray<NewtonBody*>		bodies;
	CArray<CObject3D*>		fadeOutBoxes;
	concurrent_queue<SSpawnTile>			boxesToSpawn;
	concurrent_queue<NewtonBody*>			boxesToRemove;

	CSprite3D			**bullets;
	int				bulletsNum;

	CCarEntity		*car;

	CMesh			*carbox;
	NewtonBody		*carboxBody;
	NewtonUserJoint	*mbcar;

	CSprite3D *spr2;
};