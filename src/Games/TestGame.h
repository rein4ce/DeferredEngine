#pragma once
#include "platform.h"
#include "Game.h"
#include "Editor.h"

class CLight;
class NewtonBody;
class NewtonUserJoint;
class COrbitController;
class CFlyController;
class CSprite3D;
class CSprite2D;
class CCarEntity;
class CGameOverMenu;
class CWaypoint;

#define MAX_GRASS		256


//////////////////////////////////////////////////////////////////////////
class CTestGame : public CGame
{
public:
	CTestGame();
	virtual ~CTestGame();

	virtual	void		Init();
	virtual void		Reset();
	virtual	void		Update(float frametime, float realtime);
		
	void				ControlCar();

	void				ShootBullet(Vector3 start, Vector3 dir, bool enemy = false);
	void				ShootBall(Vector3 start, Vector3 dir, float force);
	void				CreateRocket(Vector3 pos, Vector3 aim);
	void				UpdateBullets();

	void				CreatePuff(Vector3 pos, int count, SRGBA color);
	void				CreateDebris(Vector3 pos, int count, SRGBA color, Vector3 velocity = Vector3(), float r = 0.2f);
	void				CreateExplosion(Vector3 pos);
	void				CreateSmallExplosion(Vector3 pos);

	void				Reload();
	void				DrawHUD();
	void				Respawn();
	void				Kill();

	void				InitGrass();
	void				DrawGrass();

	void				ExplodeTiles(Vector3 pos, float radius);
	void				CheckWinCondition();

public:
	CLight				*pLights[4];
	CCarEntity			*pCar;

	bool				flyMode;

	CObject3D			*pHitBall;

	COrbitController	*pOrbitController;
	CFlyController		*pFlyController;

	CArray<CSprite3D*>	bullets;
	CArray<CEntity*>	waypoints;
	
	int					ammo;
	bool				reloading;
	float				reloadingStart;
	
	CSprite2D			*pAmmoHUD;
	CSprite2D			*pHPHUD;	

	int					healthPacksNum;
	int					hp;

	CGameOverMenu		*pMenu;
	EEditorMode::Type	mode;

	CParticle*			pGrass[MAX_GRASS];			
};