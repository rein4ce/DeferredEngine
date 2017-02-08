#include "platform.h"
#include "TestGame.h"
#include "Game.h"
#include "CVar.h"
#include "Level.h"
#include "Scene.h"
#include "Camera.h"
#include "Tileset.h"
#include "Controllers.h"
#include "Engine.h"
#include "Renderer.h"
#include "VGUI.h"
#include "Sprite.h"
#include "ModelLoader.h"
#include "Level.h"
#include "Tileset.h"
#include "Collision.h"
#include "Primitives.h"
#include "Movement.h"
#include "Tileset.h"
#include "Game.h"
#include "Player.h"
#include "Resources.h"
#include "Material.h"
#include "Geometries.h"
#include "ModelLoader.h"
#include "Physics.h"
#include "json_spirit.h"
#include "Car.h"
#include "ParticleSystem.h"
#include "Rocket.h"
#include "Terrain.h"
#include "Object3D.h"
#include "Level.h"
#include "Turret.h"
#include "SpawnPoint.h"
#include "ConsolePanel.h"
#include "Entities.h"
#include "GameOverMenu.h"
#include "Input.h"
#include "Editor.h"
#include "..\NewtonWin-2.30\sdk\Newton.h"
#include "Utils.h"
#include "Light.h"
#include "Mesh.h"
#include "Vector.h"




CVar cv_fogR("fogr", "255", FCVAR_ARCHIVE);
CVar cv_fogG("fogg", "255", FCVAR_ARCHIVE);
CVar cv_fogB("fogb", "255", FCVAR_ARCHIVE);
CVar cv_fogA("foga", "255", FCVAR_ARCHIVE);
CVar cv_fogNear("fognear", "55", FCVAR_ARCHIVE);
CVar cv_fogFar("fogfar", "255", FCVAR_ARCHIVE);


CTestGame::CTestGame()
{
	flyMode = false;
	pAmmoHUD = NULL;
	pHPHUD = NULL;
	pCar = NULL;
	pHitBall = NULL;
	pOrbitController = NULL;
	pFlyController = NULL;	
	healthPacksNum = 0;
	pMenu = NULL;
}

CTestGame::~CTestGame()
{
	
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::Init()
{
	CGame::Init();

	// scene
	pScene->ambientColor = SRGBA(200,200,255,255);
	pScene->CreateSkybox("skybox");
	pScene->fog = new SFog(SRGBA(255,245,233, 130), 450, 1200);
		
	// Terrain
	gTerrain.Load("textures/crater.png"); 
	CObject3D *pTerrainMesh = gTerrain.pMesh;	
	pTerrainMesh->castShadow = false;
	pScene->Add(pTerrainMesh);

	
	// Level
	pLevel = new CLevel(pScene, pWorld);
	gEditor.Load("C:/Coding/Void/bin/1.map");

	// Grass effects
	InitGrass();

	// Debug Hit ball
	pHitBall = new CMesh( new CSphereGeometry(0.1f) );
	pScene->Add(pHitBall);	

	// Car
	pCar = new CCarEntity();
	AddEntity(pCar);
	pCar->AddToScene(pScene);			// add wheels to scene
	
	
	
	// lights
	pLights[0] = new CDirectionalLight( Vector3(1,-1,1), SRGBA(155,155,144,255));
	pLights[0]->castShadow = true;	
	float aspect = (float)gEngine.width / gEngine.height;
	pLights[0]->width = 100.0f;//30.0f;
	pLights[0]->height = pLights[0]->width / aspect;

	pLights[1] = new CPointLight( Vector3(2,1,2), YELLOW, 5.5f);		
	pLights[1]->intensity = 1.0f;
	pLights[1]->overbright = true;

	pLights[3] = new CPointLight( Vector3(3,1,2), RED, 2.0f);		
	pLights[3]->intensity = 2.5f;
	pLights[3]->overbright = true;

	pLights[2] = new CDirectionalLight( Vector3(1,-1,-1), SRGBA(0,0,233,255));

	pScene->Add(pLights[0]);
	pScene->Add(pLights[1]);

	CMesh *pSphere = new CMesh( new CSphereGeometry(0.15f) );
	
	// camera controller for the car
	pFlyController = new CFlyController(pCamera);
	pCamera->SetPosition(4, 3, 4);
	pCamera->LookAt(0, 1, 0);
	pOrbitController = new COrbitController(pCamera, pCar, Vector3(+45,45,0), 5, Vector3(0,1,0));	
	pCameraController = pOrbitController;		// default orbit camera


	// UI
	pMenu = new CGameOverMenu();	
	mode = EEditorMode::Tiles;
	
	healthPacksNum = 0;	
	Respawn();

	gRenderer.AddSpriteRect(CTexture::Get("textures/darken2.png"), 0, 0, gEngine.width, gEngine.height, SRGBA(255,255,255,50));

	waypoints.Purge();
	FindEntitiesByType("Waypoint", OUT waypoints);	
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::Reset()
{
	CGame::Reset();
	gTerrain.Release();	
	if (pLevel) pLevel->Reset();

	SAFE_DELETE(pMenu);
}


//////////////////////////////////////////////////////////////////////////
// Do car controls
//////////////////////////////////////////////////////////////////////////
void CTestGame::ControlCar()
{
	pCar->ControlCar(frametime);
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::Update( float frametime, float realtime )
{
	CGame::Update(frametime, realtime);

	UpdateBullets();
	DrawHUD();

	DrawGrass();

	// update fog
	pScene->fog->color = SRGBA(
		cv_fogR.GetInt(),
		cv_fogG.GetInt(),
		cv_fogB.GetInt(),
		cv_fogA.GetInt()
		);
	pScene->fog->near = cv_fogNear.GetFloat();
	pScene->fog->far = cv_fogFar.GetFloat();

	// store all the waypoints
	waypoints.Purge();
	FindEntitiesByType("Waypoint", OUT waypoints);	

	pMenu->Update(frametime);	

	flyMode = gEditor.show;//cv_fly.GetInt() == 1;
	cv_fly.SetValue(flyMode);

	if (flyMode)
		pCameraController =  pFlyController;
	else
		pCameraController =  pOrbitController;
	
	// TEMP: spawn medkits
	static float nextMedKitSpawn = realtime + frand(1, 5);
	if (hp < 100 && realtime > nextMedKitSpawn && healthPacksNum < 3)
	{
		nextMedKitSpawn = realtime + frand(1, 5);
		CHealthPack *hpack = new CHealthPack();
		hpack->SetPosition(Vector3(frand(-50,+50), 15, frand(-50,+50)));
		healthPacksNum++;
		AddEntity(hpack);
	}
	

	cv_editor.SetValue(flyMode ? "1" : "0");

	// If not flying, perform car controls
	if (!flyMode && !gEditor.show)
	{
		pHitBall->visible = false;		

		// If we are killed, do not perform car controls
		if (hp <= 0)
		{
			Kill();
		}
		else
		{
			ControlCar();

			// Turret aiming
			Vector3 target = (pCamera->GetWorldPosition() + pCamera->forward * 1000.0f) ;
			SRayCastResult rcr;
			bool hit = CastRay(pCamera->GetWorldPosition(), target, RC_TERRAIN | RC_VOXEL, rcr );
			pCar->AimAt(hit ? rcr.hitPos : target, frametime);

			pHitBall->SetPosition(rcr.hitPos);
			
			// Firing bullets
			static float nextFireTime = 0;
			float fireDelay = 0.1f;
			if (Keydown(K_MOUSE1) && nextFireTime <= realtime && ammo > 0 && !reloading)
			{			
				ammo--;
				if (ammo == 0) Reload();

				nextFireTime = realtime + fireDelay;
				Vector3 barrelEnd = pCar->pBarrel->GetWorldVector(Vector3(0,0.03f,0.8f));
				ShootBullet(barrelEnd, pCar->pBarrel->forward);

				float puffSpeed = 0.35;

				// add puff effect
				for (int i = 0; i<3; i++)
				{					
					static CTexture *puffTex = CTexture::Get("particles/explosion4.dds");
					CParticle *p = new CParticle(puffTex);
					p->size = Vector2(0.15, 0.15);
					p->position = barrelEnd;
					p->velocity = Vector3( frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed)+1, frand(-puffSpeed,puffSpeed) );
					p->lifetime = 0.25;
					p->color = SRGBA(155,155,155,200);
					p->sizeVel = Vector2(1,1);
					p->colorChange = SRGBA(255,255,255,0);
					particles->Add(p);
					p->depthWrite = false;
				}

				// add muzzle flash
				static CTexture *muzzleTex[] = {
					CTexture::Get("particles/flame1.dds"),
					CTexture::Get("particles/explosion1.dds")				
				};

				{
					CParticle *p = new CParticle(muzzleTex[rand()%2]);
					p->size = Vector2(0.7, 0.7);
					p->position = barrelEnd+Vector3(0,0.1f,0.1f);								
					p->lifetime = 0.03;
					p->additive = true;
					particles->Add(p);
					p->depthWrite = false;
				}
			}


			// Launching rockets
			static float lastRocketFire = -100;

			if (gInput.WasKeyPressed(K_MOUSE2) )
			{
				if (lastRocketFire + 0 < realtime)
				{
					lastRocketFire = realtime;

					if (!hit)
					{
						CreateRocket(pCar->pBarrel->GetWorldPosition() + pCar->pBarrel->up/4, target);
					}
					else
					{
						CreateRocket(pCar->pBarrel->GetWorldPosition() + pCar->pBarrel->up/4, rcr.hitPos );
					}

				}
			}
		}
	}

	gEditor.Frame(frametime, realtime);
}



//////////////////////////////////////////////////////////////////////////
void CTestGame::ShootBall( Vector3 start, Vector3 dir, float force )
{
	EnterCriticalSection(&renderCS);
	dir.Normalize();
	NewtonBody *box = AddSphere(pScene, pWorld, start, 0.05f, 5);
	CObject3D *obj = (CObject3D*)NewtonBodyGetUserData(box);
	obj->color = SRGBA(33,33,33,255);
	NewtonBodyAddImpulse(box, &(dir*force)[0], &(start + dir)[0]);
	LeaveCriticalSection(&renderCS);	
}



//////////////////////////////////////////////////////////////////////////
void CTestGame::ShootBullet( Vector3 start, Vector3 dir, bool enemy)
{
	static CTexture *fxTex = CTexture::Get("textures/fx.png");

	CSprite3D *bullet = new CSprite3D(fxTex, 0.2f, 2.0f, false);
	float margin = 0.1f;
	bullet->texcoord[0] = Vector2(1-margin,1);
	bullet->texcoord[1] = Vector2(0.75+margin,0.75);
	bullet->locked = true;
	bullet->lockedAxis = pCamera->forward;
	bullet->color = enemy ? SRGBA(255,200,70,255) : YELLOW;
	bullet->enemy = enemy;

	bullet->SetPosition(start);

	float s = cv_bulletScatter.GetFloat()/2;
	dir.x += frand(-s, s);
	dir.y += frand(-s, s);
	dir.z += frand(-s, s);

	bullet->velocity = dir.Normalize() * cv_bulletSpeed.GetFloat() * (enemy ? 0.3f : 1);

	bullets.AddToTail(bullet);
	pScene->Add(bullet);	
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::UpdateBullets()
{
	for (int i=0; i<bullets.Size(); i++)
	{
		CSprite3D *bullet = bullets[i];

		Vector3 move = bullet->velocity * frametime;
		Vector3 start = bullet->GetPosition();
		bullet->Move(move.x, move.y, move.z);
		bullet->velocity.y -= frametime * cv_bulletGravity.GetFloat();

		bullet->lockedAxis = bullet->velocity;
		bullet->lockedAxis.Normalize();

		Vector3 v = bullet->velocity;
		v.Normalize();
		gRenderer.AddLine(bullet->GetPosition(), bullet->GetPosition() + v, SRGBA(255,240,210,10));
		

		SRayCastResult rc;
		if (bullet->GetWorldPosition().y < 0 || CastRay(start, start+move, RC_TERRAIN | RC_VOXEL | RC_OBJECTS, rc)) 
		{
			// if we hit the car, ignore
			if (rc.type == RC_OBJECTS)
			{
				if (!bullet->enemy && rc.object == pCar) continue;
				if (bullet->enemy && rc.object->GetType() == "Turret") continue;
				if (bullet->enemy && rc.object == pCar)
				{
					hp -= 2;
				}
			}

			//pScene->RemoveObject(bullet);
			pScene->RemoveObject(bullet);
			bullets.RemoveAt(i--);

			// spawn puff
			CreatePuff(rc.hitPos, 5, WHITE);			

			// if it's a tile, damage it
			if (rc.type == RC_VOXEL && !bullet->enemy)
			{
				int hp = rc.tile->hp;
				hp -= 25;

				// destroy tile
				if (hp <= 0)
				{
					rc.tile->type = 0;
					pLevel->UpdateTile(rc.tilePos.x, rc.tilePos.y, rc.tilePos.z);
					CreateDebris(rc.hitPos, 10, SRGBA(245,185,155,255));
				}
				else
				{
					rc.tile->hp = (byte)hp;
					CreateDebris(rc.hitPos, 3, SRGBA(245,185,155,255));
				}					
			}

			// if we hit another object, color it
			if (rc.type == RC_OBJECTS && !bullet->enemy)
			{
				if (rc.object->destructible)
				{
					rc.object->hp -= 5;						
				}
			}

			continue;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CTestGame::CreateRocket( Vector3 pos, Vector3 aim )
{
	CRocket *rocket = new CRocket();
	rocket->SetPosition(pos);
	rocket->LookAt(aim);
	rocket->velocity = (aim - pos).Normalize() * 1.0f;	
	rocket->velocity.y += 2;
	//rocket->acceleration = dir *15.0f; */
	rocket->followTarget = true;
	rocket->targetPos = aim;
	rocket->pTarget = pHitBall;

	this->AddEntity(rocket);
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::CreatePuff( Vector3 pos, int count, SRGBA color )
{
	float puffSpeed = 0.5;

	// add puff effect
	for (int i = 0; i<count; i++)
	{					
		static CTexture *puffTex = CTexture::Get("particles/explosion4.dds");
		CParticle *p = new CParticle(puffTex);
		p->size = Vector2(0.35, 0.35);
		p->position = pos;
		p->depthWrite = false;
		p->velocity = Vector3( frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed) );
		p->lifetime = 1;
		p->color = color;
		p->colorChange = p->color;
		p->colorChange.a = 0;
		p->colorTime = 1;
		p->sizeVel = Vector2(0.85,0.85);
		particles->Add(p);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::CreateDebris( Vector3 pos, int count, SRGBA color, Vector3 velocity, float r )
{
	float puffSpeed = 2.5;
	static CTexture *puffTex = CTexture::Get("textures/fx.png");

	Vector2 uv[2] = {
		Vector2(0.5f,0.5f),
		Vector2(0.5f+1.0f/12,0.5f+1.0f/12)
	};

	// add puff effect
	for (int i = 0; i<count; i++)
	{							
		CParticle *p = new CParticle(puffTex);
		p->size = Vector2(0.35, 0.35);
		p->position = pos + Vector3(frand(-r,+r),frand(-r,+r),frand(-r,+r));
		p->depthWrite = false;
		p->velocity = Vector3( frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed) ) + velocity;
		p->lifetime = 5;
		p->color = color;
		p->colorChange = p->color;
		p->colorChange.a = 255;
		p->colorTime = 5;
		p->acceleration = Vector3(0,GRAVITY,0);

		int typeX = rand() % 3;
		int typeY = rand() % 3;
		p->texcoord[0] = uv[0] + Vector2(typeX * 1.0f/12, typeY * 1.0f/12);
		p->texcoord[1] = uv[1] + Vector2(typeX * 1.0f/12, typeY * 1.0f/12);

		particles->Add(p);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::DrawHUD()
{
	if (pMenu->showing) return;
	if (cv_fly.GetInt() == 1) return;

	// Draw ammo
	int maxammo = cv_maxammo.GetInt();
	float width = 0;
	static CTexture *texAmmo = CTexture::Get("textures/ammo.png");

	int x = gEngine.width - maxammo * texAmmo->width - 10;
	int y = gEngine.height - texAmmo->height - 10;

	if (!pAmmoHUD)
	{
		pAmmoHUD = new CSprite2D();
		pAmmoHUD->pTexture = texAmmo;
		pAmmoHUD->texCoords[1].x = ammo;
		pAmmoHUD->scale.x = ammo;
		gRenderer.AddSprite(pAmmoHUD);
	}

	if (reloading)
	{
		width = (realtime - reloadingStart) / cv_reloadtime.GetFloat();
		ammo = (int)(width * maxammo);
		if (ammo >= maxammo)
		{
			ammo = maxammo;
			reloading = false;
		}
	}
	
	pAmmoHUD->position = Vector2(x,y);
	pAmmoHUD->color = reloading ? SRGBA(255,255,255,100) : WHITE;
	pAmmoHUD->scale.x = ammo;
	pAmmoHUD->texCoords[1].x = ammo;		// update the width of the ammo bar

	// Draw health
	static CTexture *texHP = CTexture::Get("textures/hp.png");
	if (!pHPHUD)
	{
		int x = +10;
		int y = gEngine.height - texAmmo->height - 10;

		pHPHUD = new CSprite2D();
		pHPHUD->pTexture = texHP;
		pHPHUD->position = Vector2(x,y);
		gRenderer.AddSprite(pHPHUD);

		gRenderer.AddSpriteRect(texHP, x, y, texHP->width*2-5, texHP->height, SRGBA(255,255,255,50));
	}

	pHPHUD->scale.x = ((float)hp / 100) * 2;
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::CheckWinCondition()
{
	CArray<CEntity*> generators;
	FindEntitiesByType("Generator", OUT generators);

	if (generators.Size() == 0)
	{
		pMenu->Show();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::Reload()
{
	reloading = true;
	ammo = 0;
	reloadingStart = realtime;
}

//////////////////////////////////////////////////////////////////////////
// Pick rnadom spawn point and drop car there with full hp
//////////////////////////////////////////////////////////////////////////
void CTestGame::Respawn()
{
	CArray<CEntity*> spawnPoints;
	FindEntitiesByType("SpawnPoint", OUT spawnPoints);

	pCar->visible = true;

	Vector3 pos;
	if (spawnPoints.Size() != 0)
	{
		int choose = rand() % spawnPoints.Size();
		spawnPoints[choose]->UpdateMatrixWorld(true);
		Vector3 pos = spawnPoints[choose]->GetWorldPosition();
		pCar->Reset(&pos);
	}

	// Reset variables
	hp = 100;
	Reload();
}

void CTestGame::CreateExplosion( Vector3 pos )
{
	static CTexture *expTex = CTexture::Get("particles/flamer.dds");

	CParticle *p = new CParticle(expTex);
	p->position = pos;
	p->size = Vector2(5,5);
	p->sizeVel = Vector2(15,15);
	p->rotationVel = 90;
	p->colorChange = SRGBA(255,255,255,100);
	p->colorTime = 10;
	p->lifetime = 5;
	p->additive = true;
	p->depthRead = false;	
	pGame->particles->Add(p);

	p = new CParticle(expTex);
	p->position = pos;
	p->size = Vector2(6,6);
	p->sizeVel = Vector2(10,10);
	p->rotationVel = 90;
	p->colorChange = SRGBA(22,22,22,100);
	p->colorTime = 5;
	p->lifetime = 5;
	p->additive = true;
	p->depthRead = false;	
	pGame->particles->Add(p);
}

void CTestGame::CreateSmallExplosion( Vector3 pos )
{
	static CTexture *expTex = CTexture::Get("particles/flamer.dds");

	CParticle *p = new CParticle(expTex);
	p->position = pos;
	p->size = Vector2(4,4);
	p->sizeVel = Vector2(8,8);
	p->rotationVel = 90;
	p->colorChange = SRGBA(255,155,155,100);
	p->colorTime = 1;
	p->lifetime = 1;
	p->additive = true;
	p->depthRead = false;	
	pGame->particles->Add(p);
}

CVar cv_explosionForce("expforce", "8.0", FCVAR_ARCHIVE);

void CTestGame::ExplodeTiles( Vector3 pos, float radius )
{
	int cx = pos.x;
	int cy = pos.y;
	int cz = pos.z;

	for (int sx = cx - radius; sx < cx + radius; sx++)
		for (int sy = cy - radius; sy < cy + radius; sy++)
			for (int sz = cz - radius; sz < cz + radius; sz++)
			{
				SMapTile *t = pLevel->GetTile(sx,sy,sz);
				if (!t) continue;
				if (Distance(sx,sy,sz,cx,cy,cz) > radius) continue;

				// spawn boxes
				if (t->type != 0)
				{
					Vector3 pos = Vector3(sx, sy, sz) + Vector3(0.5,0.5,0.5) + Vector3(pLevel->offset);
					Vector3 force = Vector3(sx, sy, sz) - Vector3(cx, cy, cz);
					force *= cv_explosionForce.GetFloat();
					CreateDebris(pos, 25, SRGBA(245,185,155,255), force, 0.5f);
				}
				
				t->type = 0;
				pLevel->UpdateTile(sx,sy,sz);	
			}
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::Kill()
{
	static float killtime = 0;
	hp = 0;

	if (killtime == 0)
	{
		CreateExplosion(pCar->GetWorldPosition());
		killtime = gEngine.realtime;
	}
	else
	{		
		// hide the vhicle after half a second
		if (gEngine.realtime > killtime + 0.5f)
		{
			pCar->visible = false;		// TODO: recursive visibility
		}

		// respawn after few seconds
		if (gEngine.realtime > killtime + 1.5f)
		{
			killtime = 0;
			Respawn();
		}
	}

}

//////////////////////////////////////////////////////////////////////////
void CTestGame::DrawGrass()
{
	const float grassRange = 20;
	Vector3 center = pCar->GetWorldPosition();
	for (int i=0; i<MAX_GRASS; i++)
	{
		CParticle *p = pGrass[i];
		float d = p->position.Distance(center);

		if (d > grassRange + 2)
		{
			float f = frand(0,6);
			p->position.x = sin(f) * grassRange * frand(0,0.9) + center.x;
			p->position.z = cos(f) * grassRange * frand(0,0.9) + center.z;			
		}
		else
		if (d > grassRange)
		{
			float f = frand(0,6);
			p->position.x = sin(f) * grassRange * 0.9 + center.x;
			p->position.z = cos(f) * grassRange * 0.9 + center.z;			
		}
		else
		{
			float min = grassRange * 0.5;
			float a = (1.0f-((d-min)/(grassRange - min)));
			p->color.a = 255.0f * max(0,min(a, 0.8));			
		}

		if (d > grassRange)
		{
			p->size.x = p->size.y = frand(0.25, 0.7);
			SRayCastResult rc;
			if (CastRay(p->position+Vector3(0,100,0), p->position-Vector3(0,100,0), RC_TERRAIN, OUT rc))
				p->position.y = rc.hitPos.y + p->size.y /2;
			p->color = SRGBA(100+rand()%28, 68+rand()%18, 8+rand()%18, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTestGame::InitGrass()
{
	static CTexture *grassTex = CTexture::Get("textures/grass.png");
	
	for (int i=0; i<MAX_GRASS; i++)
	{
		CParticle *p = new CParticle(grassTex);
		p->position = Vector3(9999,0.25,9999);
		p->locked = true;
		p->lockedAxis = Vector3(0,1,0);	
		p->size = Vector2(0.5, 0.5);
		pGrass[i] = p;
		p->depthWrite = false;
		particles->Add(p);
	}
}