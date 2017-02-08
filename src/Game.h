#pragma once
#include "platform.h"
#include "Utils.h"
#include "SlotArray.h"
#include "dContainers\dThread.h"
#include "Vector.h"
#include "Array.h"

class CVar;
class CScene;
class CCamera;
class CPerspectiveCamera;
class CController;
class CPlayerEntity;
class NewtonWorld;
class NewtonBody;
class CParticle;
class CEntity;
class CParticleSystem;
class CLevel;

extern CVar cv_gravity;

#define MAX_ENTITIES		1024
#define MAX_PARTICLES		500000

class CGame;
class CObject3D;
struct SMapTile;
class CVar;

extern CVar cv_fly;
extern CVar cv_maxammo;
extern CVar cv_reloadtime;
extern CVar cv_bulletGravity;
extern CVar cv_bulletSpeed;
extern CVar cv_bulletScatter;

// Function pointer that will handle physics
template <class T>
struct PhysicsThreadFn
{
	PhysicsThreadFn(T *target = nullptr) : target(target) { };
	void operator()(float frametime) { if (target) target->UpdatePhysics(frametime); }

private:
	T *target = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// Types of elements to cast ray versur
//////////////////////////////////////////////////////////////////////////
#define RC_TERRAIN				(1<<0)
#define RC_VOXEL				(1<<1)
#define RC_OBJECTS				(1<<2)


//////////////////////////////////////////////////////////////////////////
// Result of casting a ray through the level
//////////////////////////////////////////////////////////////////////////
struct SRayCastResult
{
	int			type;			// what did the ray collide with first?
	Vector3		hitPos;			// point of intersection

	CEntity		*object;		// mesh
	SMapTile	*tile;			// map tile
	Vector3		tilePos;		// position of the map tile
	Vector2		terrainTile;	// terrain tile
};

//////////////////////////////////////////////////////////////////////////
class CPhysicsThread : public dThread
{
public:
	CPhysicsThread();
	~CPhysicsThread() {};

	double GetTime();
	virtual unsigned RunMyTask();

	int						framesPerSecond;
	PhysicsThreadFn<CGame>	pPhysicsCallback;
};


//////////////////////////////////////////////////////////////////////////
class CGame : public CPhysicsThread
{
public:
	CGame();
	virtual ~CGame();

	virtual void			Init();
	virtual void			PostInit();
	virtual void			Reset();	

	virtual void			Update(float frametime, float realtime);
	virtual void			Render(float frametime, float realtime);


	void					AddEntity(CEntity *entity);
	void					RemoveEntity(CEntity *entity);
	void					RemoveEntity(int id);
	CEntity*				GetEntityById(int id);	

	void					UpdateEntities(float frametime, float realtime);
	virtual void			UpdatePhysics(float frametime);

	bool					CastRay(Vector3 &start, Vector3 &end, int type, OUT SRayCastResult &result);

	void					Save(string filename);
	bool					Load(string filename);

	void					SaveEntities(ofstream &fs);
	void					LoadEntities(ifstream &fs);

	void					FindEntitiesByType(string name, OUT CArray<CEntity*> &ret);
	void					FindEntitiesByTypeInRange(string name, Vector3 pos, float radius, OUT CArray<CEntity*> &ret);

protected:	
	void					InitPhysicsThread();	

public:	
	static int				nextEntityId;
	CArray<CEntity*>		entities;		

	CParticleSystem			*particles;

	CScene					*pScene;
	CCamera					*pCamera;
	CController				*pCameraController;

	CPlayerEntity			*pPlayer;
	NewtonWorld				*pWorld;
	
	float					frametime;
	float					realtime;

	CObject3D				*pEditTile;				// tile mesh that is about to be placed
};

extern CGame		*pGame;
extern CVar			cv_editor;
extern CVar			cv_splash;

NewtonBody* AddSphere(CScene *pScene, NewtonWorld *pWorld, Vector3 pos, float radius, float mass);
NewtonBody* AddBox(CScene *pScene, NewtonWorld *pWorld, Vector3 pos, Vector3 size, Vector3 rot, float mass = 0);
void BoxGravityCallback (const NewtonBody* body, float timestep, int threadIndex);