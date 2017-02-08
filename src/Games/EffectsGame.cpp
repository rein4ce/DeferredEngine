#include "platform.h"
#include "EffectsGame.h"
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

#include <Newton.h>
#include "Light.h"
#include "keydefs.h"
#include "Input.h"
#include "Mesh.h"
#include "Utils.h"
#include "Vector.h"


extern int gLevelChunksMaterialID;

CEffectsGame* CEffectsGame::pInstance = NULL;


#define GRAVITY -10.0f

CVar cv_speed = CVar("speed", "15.0", FCVAR_ARCHIVE);
CVar cv_rspeed = CVar("rspeed", "50.0", FCVAR_ARCHIVE);

void CarBoxCallback(const NewtonBody* body, float timestep, int threadIndex)
{
	float mass, ix, iy, iz;

	NewtonBodyGetMassMatrix(body, &mass, &ix, &iy, &iz);
	Vector4 gravityForce = Vector4(0.0f, mass * GRAVITY, 0.0f, 1.0f);

	CMesh *object = (CMesh*)NewtonBodyGetUserData(body);
	Vector3 force;

	if (Keydown('w')) force = object->forward;
	if (Keydown('s')) force = -object->forward;

	force *= cv_speed.GetFloat();	// speed

	force *= mass;
		
	NewtonBodySetForce(body, &force[0]);	

	float torqueForce = cv_rspeed.GetFloat();
	Vector3 torque;
	if (Keydown('a')) torque.y -= torqueForce;
	if (Keydown('d')) torque.y += torqueForce;
		
	NewtonBodySetOmega(body, &torque[0]);
	
	int sleep = NewtonBodyGetSleepState(body);
	object->color = sleep == 1 ? GREEN : RED;
}


void CEffectsGame::UpdatePhysics(float frametime)
{
	static LARGE_INTEGER PerformanceFrequency;
	static LARGE_INTEGER MSPerformanceFrequency;
	static LARGE_INTEGER ClockStart;

	if( !PerformanceFrequency.QuadPart )
	{
		QueryPerformanceFrequency(&PerformanceFrequency);
		MSPerformanceFrequency.QuadPart = PerformanceFrequency.QuadPart / 1000;
		QueryPerformanceCounter(&ClockStart);
	}
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter( &CurrentTime );
	double fRawSeconds = (double)( CurrentTime.QuadPart - ClockStart.QuadPart ) / (double)(PerformanceFrequency.QuadPart);
	float newTime = fRawSeconds;
		
	static float accumulator = 0;
	static double oldTime = GetFloatTime();
		
exit:		

	int boxCounter = 0;

	// spawn and remove boxes
	EnterCriticalSection(&renderCS);

	while (!boxesToSpawn.empty())
	{
		SSpawnTile s;			
		boxesToSpawn.wait_and_pop(s);

		NewtonBody *box = AddBox(pScene, pWorld, s.pos+Vector3(0.5, 0.5, 0.5), Vector3(0.95f,0.95f,0.95f), Vector3(), 100);

		// add some velocity
		NewtonBodySetVelocity(box, &s.vel[0]);

		if (++boxCounter > 5) break;		// don't spawn more per one frame than cap
	}

	while (!boxesToRemove.empty())
	{
		NewtonBody *body;
		CEffectsGame::pInstance->boxesToRemove.wait_and_pop(body);
		CMesh *object = (CMesh*)NewtonBodyGetUserData(body);

		pScene->RemoveObject(object);
		NewtonDestroyBody(CEffectsGame::pInstance->pWorld, body);
	}

	LeaveCriticalSection(&renderCS);

	static float physAcc = 0;
	physAcc += frametime;
	if (physAcc > 1.0f/60)
	{
		EnterCriticalSection(&renderCS);
		NewtonUpdate(pWorld, frametime);
		LeaveCriticalSection(&renderCS);
		physAcc = 0;
	}	
}



CEffectsGame::CEffectsGame()
{
	pWorld = NULL;
	bullets = NULL;
	car = NULL;
	pWorld = NULL;
	pFloorCollision = NULL;
	pFloorBody = NULL;
	pFloor = NULL;
	pInstance = this;
	pLevel = NULL;
	bulletsNum = 0;
}

CEffectsGame::~CEffectsGame()
{
		
}


static unsigned DropDownConvexCastCallback (const NewtonBody* body, const NewtonCollision* collision, void* userData)
{
	// this convex cast have to skip the casting body
	NewtonBody* me = (NewtonBody*) userData;
	return (me == body) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////
// Position the body precisely
//////////////////////////////////////////////////////////////////////////
void DropDownBox(NewtonBody *body)
{
	float matrix[16];
	NewtonBodyGetMatrix(body, matrix);
	Matrix4 m = Matrix4(matrix);

	Vector3 pos = m.GetPosition();
	pos.y += 1.0f;
	m.SetPosition(pos);
	Vector3 p = pos;
	p.y -= 20;

	// cast collision shape within +20 -20 y range
	NewtonWorld *world = NewtonBodyGetWorld(body);
	NewtonCollision *collision = NewtonBodyGetCollision(body);

	float param;
	NewtonWorldConvexCastReturnInfo info[16];
	m.FlattenToArray(matrix);
	NewtonWorldConvexCast(world, matrix, &p[0], collision, &param, body, DropDownConvexCastCallback, info, 16, 0);

	m = Matrix4(matrix);
	pos = m.GetPosition();
	m.SetPosition(pos + Vector3(0, (p.y - pos.y) * param, 0) );

	m.FlattenToArray(matrix);
	NewtonBodySetMatrix(body, matrix);
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::RemoveBodies()
{
	for (int i=0; i<bodies.Size(); i++)
	{
		NewtonDestroyBody( pWorld, bodies[i]);
		CObject3D *object = (CObject3D*)NewtonBodyGetUserData(bodies[i]);
		pScene->Remove(object);
	}

	bodies.Purge();
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::ReloadGeometry()
{	
	RemoveBodies();

	ifstream is("level.json");
	json_spirit::mValue array;
	json_spirit::read(is, array);

	const json_spirit::mArray &boxes = array.get_array();
	int num = boxes.size();
	
	for (int i=0; i<num; i++)
	{
		const json_spirit::mObject &box = boxes[i].get_obj();

		
		json_spirit::mArray position = box.find("pos")->second.get_array();
		Vector3 pos = Vector3(
			position[0].get_real(),
			position[1].get_real(),
			position[2].get_real()
			);

		json_spirit::mArray size = box.find("size")->second.get_array();
		Vector3 s = Vector3(
			size[0].get_real(),
			size[1].get_real(),
			size[2].get_real()
			);

		json_spirit::mArray rotation = box.find("rotation")->second.get_array();
		Vector3 rot = Vector3(
			rotation[0].get_real(),
			rotation[1].get_real(),
			rotation[2].get_real()
			);

		bodies.AddToTail( AddBox(pScene, pWorld, pos, s, rot) );
	}
}


//////////////////////////////////////////////////////////////////////////
void CEffectsGame::CreateCar()
{

}


//////////////////////////////////////////////////////////////////////////
void CEffectsGame::Init()
{
	CGame::Init();

	CTileset::Init();

	// create drivable car
	car = new CCarEntity();	
	car->Move(-15,0.5f,-10);
	AddEntity(car);
	car->AddToScene(pScene);
	pCameraController = new COrbitController( pCamera, car, Vector3(+45,45,0), 5, Vector3(0,2,1) );

	bullets = new CSprite3D*[MAX_BULLETS];
	for (int i=0; i<MAX_BULLETS; i++) bullets[i] = NULL;
	
	gInput.SetCenterCursor(true);
	
	CreateScene();

	ContinueExecution();
}



//////////////////////////////////////////////////////////////////////////
void CEffectsGame::ApplyForceAndTorqueCallback (const NewtonBody* body, float timestep, int threadIndex)
{
	float mass, ix, iy, iz;
	NewtonBodyGetMassMatrix(pInstance->pBoxBody, &mass, &ix, &iy, &iz);
	Vector4 gravityForce = Vector4(0.0f, mass * GRAVITY, 0.0f, 1.0f);

	if (gInput.WasKeyPressed(K_MOUSE1))
	{
		
		Vector4 rnd = Vector4(frand(-1,1), 1, frand(-1,1), 0);
		rnd *= 100.0f;
		gravityForce += rnd;
	}


	NewtonBodySetForce(pInstance->pBoxBody, (float*)&gravityForce);
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::Reset()
{
	RemoveBodies();
	SAFE_DELETE(pLevel);

	if (bullets) SAFE_DELETE_ARRAY(bullets);

	CGame::Reset();
}

void GetMeshesList(CObject3D *object, OUT CArray<CObject3D*> &meshes)
{
	if (object->geometry) meshes.AddToTail(object);
	for (int i=0; i<object->children.Size(); i++)
		GetMeshesList(object->children[i], meshes);
}



//////////////////////////////////////////////////////////////////////////
void CEffectsGame::DoPicking()
{
	bool hit = false;
	Vector3 tileHitPos;
	SMapTile *tileHit;
	Vector3 start = pCamera->GetWorldPosition();
	Vector3 end = start + pCamera->forward * 1000.0f;
	Vector3 pos;

	pLevel->UnHighlightTile();

	// find tile we are pointing at
	if (hit = pLevel->CastRay(start, end, OUT tileHitPos, OUT &tileHit, OUT &pos))
	{
		pLevel->HighlightTile(tileHitPos.x, tileHitPos.y, tileHitPos.z);		
	}

	Vector3 hitPos = tileHitPos + pLevel->offset + Vector3(0.5, 0.5, 0.5);

	// aim the turret
	if (hit)
	{
		car->AimAt(hitPos, frametime);
	}
	else
	{
		Vector3 target = (pCamera->GetWorldPosition() + pCamera->forward * 1000.0f);
		car->AimAt(target, frametime);
	}

	static float force = 10;
	if (gInput.WasKeyPressed(K_PGUP)) force += 5.0f;
	if (gInput.WasKeyPressed(K_PGDN)) force -= 5.0f;

	//car->pBarrel->DrawAxis();

	// shooting - delete tile
	if (gInput.IsKeydown(K_MOUSE1))
	{
		static float delay = 0;
		delay += frametime;
		if (delay > 0.05f)
		{
			
			// shoot a bullet
			CreateBullet(car->pBarrel->GetWorldPosition(), car->pBarrel->up);

			float puffSpeed = 0.35;

			// add puff effect
			for (int i = 0; i<3; i++)
			{					
				static CTexture *puffTex = new CTexture("particles/explosion4.dds");
				CParticle *p = new CParticle(puffTex);
				p->size = Vector2(0.15, 0.15);
				p->position = car->pBarrel->GetWorldPosition() + car->pBarrel->up/2;
				p->velocity = Vector3( frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed)+1, frand(-puffSpeed,puffSpeed) );
				p->lifetime = 0.25;
				p->color = SRGBA(255,255,255,50);
				p->sizeVel = Vector2(1,1);
				p->colorChange = SRGBA(255,255,255,0);
				particles->Add(p);
			}

			// add muzzle flash
			static CTexture *muzzleTex[] = {
				new CTexture("particles/flame1.dds"),
				new CTexture("particles/explosion1.dds")				
			};
		
			{
				CParticle *p = new CParticle(muzzleTex[rand()%2]);
				p->size = Vector2(0.7, 0.7);
				p->position = car->pBarrel->GetWorldPosition() + car->pBarrel->up/2;								
				p->lifetime = 0.01;
				p->additive = true;
				particles->Add(p);
			}

			
			delay = 0;

			// if hit a tile, destroy it
			if (hit)
			{
				EnterCriticalSection(&renderCS);
				tileHit->type = 0;
				LeaveCriticalSection(&renderCS);

				SMapChunk *chunk = pLevel->GetChunk(
					floor(tileHitPos.x/SMapChunk::Size),
					floor(tileHitPos.y/SMapChunk::Size),
					floor(tileHitPos.z/SMapChunk::Size) );

				chunk->dirtyBody = true;
				chunk->dirty = true;

				SSpawnTile s;
				s.pos = tileHitPos;
				s.vel = pCamera->forward * force;
				boxesToSpawn.push(s);
			
				// create a physical entity in this place
				NewtonBody *box = AddBox(pScene, pWorld, hitPos, Vector3(0.95f,0.95f,0.95f), Vector3(), 100);

				// add some velocity
				Vector3 vel = pCamera->forward * force;
				NewtonBodySetVelocity(box, &vel[0]);		
			}
		}
	}

	static float lastRocketFire = -100;

	if (gInput.WasKeyPressed(K_MOUSE2) )
	{
		float r = 3;
		int x = tileHitPos.x;
		int y = tileHitPos.y;
		int z = tileHitPos.z;

		if (lastRocketFire + 0 < realtime)
		{
			Debug("Create rocket!");
			lastRocketFire = realtime;

			if (!hit)
			{
				Vector3 dir = (pos - (car->pBarrel->GetWorldPosition() + Vector3(0, -10, 0))).Normalize();
				if (dir.y < -0.25)
				{
					dir.y = -0.25;
					dir.Normalize();
				}
				CreateRocket(car->pBarrel->GetWorldPosition() + car->pBarrel->up / 2, dir);//car->pBarrel->up);
			}
			else
			{
				Vector3 dir = (pos-(car->pBarrel->GetWorldPosition() + Vector3(0,-10,0))).Normalize();
				if (dir.y < -0.25) 
				{
					dir.y = -0.25;
					dir.Normalize();
				}
				CreateRocket(car->pBarrel->GetWorldPosition() + car->pBarrel->up/2, dir );
			}
		}
	}

	// show car meshes
	if (car)
	{
		CArray<CObject3D*> meshes;
		GetMeshesList(car, meshes);

		float minT = 99999.0f;

		static int current = 0;
		if (KeyPressed(']')) { meshes[current]->color = WHITE; current++; if (current == meshes.Size()) current = 0; meshes[current]->color = RED;  }
		if (KeyPressed('[')) { meshes[current]->color = WHITE; current--; if (current < 0) current = meshes.Size()-1; meshes[current]->color = RED; }

		car->aimTarget = end;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::FadeOutBoxes()
{
	for (int i=0; i<fadeOutBoxes.Size(); i++)
	{
		if (!fadeOutBoxes[i])
		{
			fadeOutBoxes.RemoveAt(i--);
			continue;
		}

		if (fadeOutBoxes[i]->killTime == 0)
		{
			fadeOutBoxes[i]->killTime = realtime + 5.0f;
		}
		else
		{
			if (fadeOutBoxes[i]->killTime < realtime)
			{
				fadeOutBoxes[i]->opacity -= frametime / 2;
				if (fadeOutBoxes[i]->opacity < 0)
				{
					fadeOutBoxes[i]->opacity = 0;
					pScene->RemoveObject(fadeOutBoxes[i]);
					fadeOutBoxes.RemoveAt(i--);
				}
			}
		}
		
	}
}

//////////////////////////////////////////////////////////////////////////
void UpdateParticle(CParticle *p)
{
	if (p->position.y < 0)
	{
		p->lifetime = -1;
		p->killtime = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::Update( float frametime, float realtime )
{
	CGame::Update(frametime, realtime);

	car->ControlCar(frametime);

	// checking if hit rays
	DoPicking();
	UpdateBullets(frametime);
	
	// car movement
	if (car)
	{
		car->moveForward = Keydown('w');
		car->moveBackward = Keydown('s');
		car->moveLeft = Keydown('a');
		car->moveRight = Keydown('d');
	}
}

#define DG_USE_NORMAL_PRIORITY_THREAD

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::CreateBoxesScene()
{
	float size = 60;

	// floor
	NewtonBody *bFloor = AddBox(pScene, pWorld, Vector3(0,-1,0), Vector3(10000,1,10000), Vector3());	
	NewtonBody *bCeiling = AddBox(pScene, pWorld, Vector3(0,size/2,0), Vector3(size,0.1,size), Vector3());	

	// walls
	AddBox(pScene, pWorld, Vector3(-size/2,size/4,0), Vector3(1,size/2,size), Vector3());	
	AddBox(pScene, pWorld, Vector3(+size/2,size/4,0), Vector3(1,size/2,size), Vector3());	
	AddBox(pScene, pWorld, Vector3(0,size/4,-size/2), Vector3(1,size/2,size), Vector3(0,90,0));	
	AddBox(pScene, pWorld, Vector3(0,size/4,+size/2), Vector3(1,size/2,size), Vector3(0,90,0));	

	// light
	pScene->ambientColor = SRGBA(50,50,60,255);

	// create block
	float bias = 1.1f;
	for (int y=0; y<6; y++)
		for (int x=0; x<8; x++)
			for (int z=0; z<8; z++)			
			{
				NewtonBody *box = AddBox(pScene, pWorld, Vector3(-x*bias,y*bias+1,z*bias), Vector3(1,1,1), Vector3(), 100);
				DropDownBox(box);
			}
	
	SRGBA white = WHITE;
	pScene->Add( new CDirectionalLight(Vector3(1,-2,0.5), white));
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::CreateScene()
{
	pScene->CreateSkybox("clear");

	NewtonBody *bFloor = AddBox(pScene, pWorld, Vector3(0,-0.5,0), Vector3(1000,1,1000), Vector3());	
	NewtonCollision *col = NewtonCreateTreeCollision(pWorld, 0);
	NewtonTreeCollisionBeginBuild(col);
	Vector3 v[4] = {
		Vector3(-1000,0.5f,-1000),
		Vector3(-1000,0.5f,+1000),
		Vector3(+1000,0.5f,+1000),
		Vector3(+1000,0.5f,-1000)
	};
	NewtonTreeCollisionAddFace(col, 4, &v[0][0], sizeof(Vector3), 1);
	NewtonTreeCollisionEndBuild(col, 0);
	NewtonBodySetCollision(bFloor, col);

	CObject3D *f = (CObject3D*)NewtonBodyGetUserData(bFloor);
	f->visible = false;

	NewtonBodySetMaterialGroupID(bFloor, gLevelChunksMaterialID);

	// floor
	static CTexture *floorTex = new CTexture("textures/512.png");
	CMaterial *floorMat = new CMaterial();
	floorMat->features = EShaderFeature::LIGHT | EShaderFeature::FOG | EShaderFeature::SHADOW | EShaderFeature::TEXTURE;
	CPlaneGeometry *floorGeom = new CPlaneGeometry(1000,1000);
	CObject3D *ground = new CMesh( floorGeom, floorMat );
	ground->geometry->materials.AddToTail(floorMat);
	floorMat->pTexture = floorTex;
	floorMat->specular = 0.25;
	floorGeom->SetTextureScale(40,40);
	ground->SetPosition(-500, 0, -500);
	pScene->Add(ground);

	

	pLevel = new CLevel(pScene, pWorld);
	pLevel->Create(128,80,128);

	CreateVoxelBuilding(10, 0, 10, 24, 24, 8, 8, 1);

	CreateVoxelBuilding(40, 0, 10, 24, 24, 8, 8, 2);

	CreateVoxelBuilding(70, 0, 10, 24, 24, 6, 8, 1);

	CreateVoxelBuilding(100, 0, 10, 24, 44, 2, 8, 2);

	CreateVoxelBuilding(90, 0, 50, 24, 16, 4, 8, 1);

	CreateVoxelBuilding(78, 0, 55, 32, 24, 5, 12, 1);

	CreateVoxelBuilding(100, 00, 90, 20, 18, 1, 12, 2);

	CreateVoxelBuilding(0, 0, 70, 48, 48, 2, 6, 1);

	int sx = 55;
	int sz = 55;
	
	pLevel->Recreate();

	for (int x=0; x<pLevel->chunksX; x++)
		for (int y=0; y<pLevel->chunksY; y++)
			for (int z=0; z<pLevel->chunksZ; z++)
			{
				pLevel->GetChunk(x,y,z)->RecreateCollision();
			}	
		
	pScene->fog = new SFog( SRGBA(172,201,241, 255), 200, 1500);
	
	// directional Sun light
	CDirectionalLight *pDirLight = new CDirectionalLight(Vector3(0,0,0), SRGBA(255,225,175,255));
	pDirLight->SetPosition(+70,90,-70);
	pDirLight->LookAt(Vector3());
	pDirLight->UpdateMatrixWorld(true);
	pDirLight->shadowNear = 20;
	pDirLight->shadowFar = 200;
	pDirLight->castShadow = true;
	float aspect = (float)gEngine.width / gEngine.height;
	pDirLight->width = 200.0f;
	pDirLight->height = pDirLight->width / aspect;
	pScene->Add( pDirLight );
	
	// ambient light
	pScene->ambientColor = SRGBA(200,200,255,255);

	pCamera->LookAt(Vector3());
}

void CEffectsGame::CreateVoxelBuilding(int posX, int posY, int posZ, int width, int depth, int storeys, int storyHeight, int tileType)
{
	for (int y = 0; y < storeys*storyHeight; y++)
		for (int x = 0; x < width; x++)
			for (int z = 0; z < depth; z++)
			{
				int block = 0;
				if (x == 0 || z == 0 || x == width - 1 || z == depth - 1) block = 1;
				if (y%storyHeight == storyHeight - 1) block = 1;
				if (x > 5 && z > 5 && x < width - 5 && z < depth - 5) block = 0;

				if (y%storyHeight > 2 && y%storyHeight <= 4)
				{
					if (x % 8 >= 2 && x % 8 < 7) block = 0;
					if (z % 8 >= 2 && z % 8 < 7) block = 0;
				}

				if ((x == 5 && z == 5) || (x == width - 5 && z == depth - 5) || (x == 5 && z == depth - 5) || (x == width - 5 && z == 5)) block = 4;


				if (block > 0)	block = tileType;
				auto tile = pLevel->GetTile(posX+x, posY+y, posZ+z);
				if (!tile) continue;
				tile->type = block;
			}

}

void CEffectsGame::CreateBuilding( float px, float py, float pz, float sizeX, float sizeY, float sizeZ )
{
	float spacing = 5; 
	float height = 2;
	float roofThickness = 0.25;
	float bias = 0.001f;


	Vector3 pos = Vector3(px,py,pz);
	for (int y=0; y<sizeY; y++)
	{
		for (int x=0; x<sizeX; x++)
			for (int z=0; z<sizeZ; z++)
			{
				float bx = ((x == 0) ? 0.2f : (x == sizeX-1 ? -0.2f : 0));
				float bz = ((z == 0) ? 0.2f : (z == sizeZ-1 ? -0.2f : 0));
				NewtonBody *pillar = AddBox(pScene, pWorld, pos+Vector3(x*spacing+bx, y*height+height/2+(y*roofThickness), bz+z*spacing), Vector3(0.25,height,0.25), Vector3(), 5000);
				DropDownBox(pillar);				
			}

			for (int x=0; x<sizeX-1; x++)
				for (int z=0; z<sizeZ-1; z++)
				{
					NewtonBody *roof = AddBox(pScene, pWorld, pos+Vector3(x*spacing+spacing/2, (y+1)*height+(y*roofThickness)+roofThickness/2, z*spacing+spacing/2), Vector3(spacing-bias,roofThickness,spacing-bias), Vector3(), 90000);
					DropDownBox(roof);				
				}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::CreateBullet( Vector3 pos, Vector3 dir )
{
	// find empty slot
	int i = 0;
	for (i=0; i<MAX_BULLETS; i++) 
		if (bullets[i] == NULL) break;

	if (i == MAX_BULLETS) return;

	static CTexture *fxTex = new CTexture("textures/fx.png");

	CSprite3D *bullet = new CSprite3D(fxTex, 0.2f, 2.0f, false);
	float margin = 0.1f;
	bullet->texcoord[0] = Vector2(1-margin,1);
	bullet->texcoord[1] = Vector2(0.75+margin,0.75);
	bullet->locked = true;
	bullet->lockedAxis = pCamera->forward;
	bullet->color = YELLOW;

	bullet->SetPosition(car->pBarrel->GetWorldPosition()/* + car->pBarrel->up * 1.0f*/);

	float s = cv_bulletScatter.GetFloat()/2;
	dir.x += frand(-s, s);
	dir.y += frand(-s, s);
	dir.z += frand(-s, s);

	bullet->velocity = dir.Normalize() * cv_bulletSpeed.GetFloat();
	
	pScene->Add(bullet);

	bullets[i] = bullet;
	bulletsNum++;
}

CVar cv_bulletforce = CVar("bulletforce", "20", FCVAR_ARCHIVE);

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::UpdateBullets( float frametime )
{
	for (int i=0; i<MAX_BULLETS; i++) 
	{
		if (bullets[i] == NULL) continue;;

		CSprite3D *bullet = bullets[i];

		Vector3 move = bullet->velocity * frametime;
		Vector3 start = bullet->GetPosition();
		bullet->Move(move.x, move.y, move.z);
		bullet->velocity.y -= frametime * cv_bulletGravity.GetFloat();

		bullet->lockedAxis = bullet->velocity;
		bullet->lockedAxis.Normalize();

		Vector3 v = bullet->velocity;
		v.Normalize();
		gRenderer.AddLine(bullet->GetPosition(), bullet->GetPosition() + v, SRGBA(255,230,180,180));


		if (bullet->GetWorldPosition().y < 0) 
		{
			RemoveBullet(i);
		}

		SMapTile *tileHit;
		Vector3 tileHitPos;
		Vector3 pos;
		bool hit = false;
		if (hit = pLevel->CastRay(start, bullet->GetPosition(), OUT tileHitPos, OUT &tileHit, OUT &pos))
		{			
				EnterCriticalSection(&renderCS);
				tileHit->type = 0;
				LeaveCriticalSection(&renderCS);

				// remove tile from the level and mark for update
				SMapChunk *chunk = pLevel->GetChunk(
					floor(tileHitPos.x/SMapChunk::Size),
					floor(tileHitPos.y/SMapChunk::Size),
					floor(tileHitPos.z/SMapChunk::Size) );

				chunk->dirtyBody = true;
				chunk->dirty = true;
				
				// add a box to spawn list
				SSpawnTile s;
				s.pos = tileHitPos;
				s.vel = v * cv_bulletforce.GetFloat();
				boxesToSpawn.push(s);			
				
				// remove bullet
				RemoveBullet(i);

				float puffSpeed = 0.5;

				// add puff effect
				for (int i = 0; i<6; i++)
				{					
					static CTexture *puffTex = new CTexture("particles/explosion4.dds");
					CParticle *p = new CParticle(puffTex);
					p->size = Vector2(1.35, 1.35);
					p->position = pos;
					p->velocity = Vector3( frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed), frand(-puffSpeed,puffSpeed) );
					p->lifetime = 1;
					//p->color = SRGBA(155,155,155,255);
					p->color = SRGB(clamp(pos.x*255.0f/32.0f,0,255)*0.9f, clamp(pos.y*255.0f/32.0f,0,255)*0.9f, clamp(pos.z*255.0f/32.0f,0,255)*0.9f);
					p->colorChange = p->color;// SRGBA(255,255,255,0);
					p->colorChange.a = 0;
					p->colorTime = 1;
					p->sizeVel = Vector2(0.85,0.85);
					particles->Add(p);
				}
		}		
	
	}

	//PrintText("Bullets: %d", bulletsNum);
}

void CEffectsGame::RemoveBullet( int i )
{
	if (!bullets[i]) return;
	pScene->RemoveObject(bullets[i]);
	bullets[i] = NULL;
	bulletsNum--;
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::CreateRocket( Vector3 pos, Vector3 dir )
{
	CRocket *rocket = new CRocket();
	rocket->SetPosition(pos);
	rocket->LookAt(pos+dir);
	rocket->velocity = dir * 1.0f;	
	rocket->velocity.y += 5;
	rocket->acceleration = dir *15.0f; 

	this->AddEntity(rocket);
}

//////////////////////////////////////////////////////////////////////////
void CEffectsGame::SpawnTile( SSpawnTile &tile )
{
	EnterCriticalSection(&renderCS);
	boxesToSpawn.push(tile);
	LeaveCriticalSection(&renderCS);
}

