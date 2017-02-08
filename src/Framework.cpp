#include "platform.h"
#include "Framework.h"
#include "VoidGame.h"
#include "CVar.h"
#include "ConsolePanel.h"
#include "cfgparser.h"
#include "Cursor.h"
#include "Renderer.h"
#include "ResourceMgr.h"
#include "Scene.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Geometries.h"
#include "Sprite.h"
#include "ModelLoader.h"
#include "Level.h"
#include "Tileset.h"
#include "Collision.h"
#include "Primitives.h"
#include "Movement.h"
#include "Tileset.h"
#include "Game.h"
#include "EffectsGame.h"
#include "TestGame.h"
#include "Engine.h"
#include "FrameGraph.h"
#include "ParticleSystem.h"
#include "EmptyGame.h"
#include "HeadGame.h"
#include "Input.h"
#include "Light.h"
#include "Utils.h"
#include "Font.h"
#include "Utils.h"
#include "VGUI.h"
#include "BlocksGame.h"

// Globalny obiekt frameworku aplikacji
CFramework			gFramework;

// Forward declarations
void QuitFunc( void );
void StartFunc( void );
void EndFunc( void );

// Komendy konsoli
CCommand cmd_Quit		( "quit", QuitFunc );
CCommand cmd_Start		( "start", StartFunc );
CCommand cmd_End		( "end", EndFunc );

CScene					*pScene;
CPerspectiveCamera		*pCamera;
CObject3D				*pCube;
CObject3D				*pGroup;
//CLight					*pLight;
CTexture				*pTexture;
CSprite3D					*spr;
CObject3D				*pWheels[4];
CGame					*pGame;


VoidFont				fontDin;


class CDemo
{
public:
	CDemo() 
	{
		scene = new CScene();		
	}
	virtual void Frame(float time, float realtime) = 0;

	CScene *scene;
};

class CEnvDemo : public CDemo
{
public:
	CEnvDemo()
	{
		scene->CreateSkybox("SwedishRoyalCastle");

		CMaterial *headMat = new CMaterial();	
		headMat->features = EShaderFeature::LIGHT;
		headMat->pEnvTexture = new CCubeTexture("skybox/SwedishRoyalCastle/castle.dds");	
		headMat->color = WHITE;
		headMat->features |= EShaderFeature::ENVMAP;

		CObject3D *head = CModelLoader::Load("models/WaltHead.obj", headMat);
		head->name = "head";
		scene->Add( pLight = new CPointLight(Vector3(3.0f,0,3.0f), WHITE, 3.5f, SRGBA(255,255,200,255)) );

		CGeometry* g = new CSphereGeometry(0.2f);
		CMaterial *ballMat = new CMaterial();
		ballMat->features = EShaderFeature::LIGHT | EShaderFeature::ENVMAP;
		ballMat->pEnvTexture = headMat->pEnvTexture;
		g->materials.AddToHead(ballMat);
		pLight->Add(new CMesh(g));

		head->SetScale(0.05f);
		head->SetPosition(0,-2,0);
		scene->Add(head);

		CMaterial *h2m = new CMaterial();	
		h2m->features = EShaderFeature::LIGHT;
		h2m->pEnvTexture = headMat->pEnvTexture;	
		h2m->features |= EShaderFeature::ENVMAP;
		h2m->reflectivity = 0.3f;
		h2m->color = SRGBA(255,99,0,255);

		CObject3D *h2 = CModelLoader::Load("models/WaltHead.obj", h2m);
		CObject3D *h3 = CModelLoader::Load("models/WaltHead.obj", headMat);
		h2->SetPosition(5,-2,0);
		h3->SetPosition(-5,-2,0);
		h2->SetScale(0.05f);
		h3->SetScale(0.05f);
		scene->Add(h2);
		scene->Add(h3);
	}

	void Frame(float time, float realtime)
	{
		pLight->SetPosition(cos(realtime)*4.0f, 0 ,sin(realtime)*4.0f);	
	}

	CLight* pLight;
};

class CEffectsDemo : public CDemo
{
public:
	CEffectsDemo()
	{
		CTexture *tex = new CTexture("textures/512.png");
		mat = new CMaterial();
		mat->pTexture = tex;
		mat->features = EShaderFeature::LIGHT | EShaderFeature::TEXTURE | EShaderFeature::FOG;

		CMesh *box = new CMesh( new CCubeGeometry(5, 5, 5));
		box->geometry->materials.AddToTail(mat);
		box->SetScale(-1);
		box->SetPosition(0,2.3f,0);
		for (int i=0; i<box->geometry->faces.Size(); i++)
		{
			box->geometry->faces[i].normal *= -1.0f;
			for (int j=0; j<3; j++)
			{
				box->geometry->faces[i].texcoord[j].y = 1.0f - box->geometry->faces[i].texcoord[j].y;
			}			
		}

		scene->Add(box);

		scene->fog = new SFog(SRGBA(0,0,0,255), 3, 8);
		scene->ambientColor = SRGBA(55,55,55,255);

		scene->Add( pLight = new CPointLight(Vector3(0,1,0), SRGBA(255,233,155,255)) );
		pLight->specular = SRGBA(255,233,155,100);
		pLight->range = 1;
		pLight->intensity = 1;
				
		mat->specular = 0.04f;

		// Smoke
		CTexture *fxTex = new CTexture("textures/fx.png");		

		float posX = 0, posY = 1, posZ = 0;
	
		// 1) Flame/Smoke
		for (int i=0; i<4; i++)
		{
			float rnd = 0.05f;
			float x = posX + frand(-rnd,rnd);
			float y = posY + frand(-rnd,rnd)-0.5f;
			float z = posZ + frand(-rnd,rnd);
			CSprite3D *spr = new CSprite3D(fxTex, 1.0f+i*0.2f, 1.0f+i*0.2f, false);
			spr->SetPosition(x,y,z);
			scene->Add(spr);
			//spr->locked = true;
			int type = i%4;
			x = type%2;
			y = type/2;
			spr->texcoord[0] = Vector2(x*0.25,y*0.25);
			spr->texcoord[1] = Vector2(x*0.25+0.25,y*0.25+0.25);
			spr->additive = true;

			spr->color = SRGB(178,95,16);;//SRGB(118,73,16);
			if (i == 0) spr->color = SRGB(255,224,222);
		}

		CMaterial *fxMat = new CMaterial();
		fxMat->pTexture = fxTex;
		fxMat->features = EShaderFeature::TEXTURE;
		fxMat->color = SRGB(255,224,147);
		fxMat->blending = EBlending::Additive;
		fxMat->transparent = true;
		CGeometry *plane = new CPlaneGeometry(0.05f, 0.8f);

		float n = 0.1f;
		plane->faces[0].texcoord[0] = Vector2(1, 1.0f - n);
		plane->faces[0].texcoord[2] = Vector2(0.75f, 0.75f + n);
		plane->faces[0].texcoord[1] = Vector2(0.75f, 1.0f - n);

		plane->faces[1].texcoord[1] = Vector2(1, 0.75f+n);
		plane->faces[1].texcoord[0] = Vector2(0.75f, 0.75f + n);
		plane->faces[1].texcoord[2] = Vector2(1, 1.0f - n);

		fxMat->doubleSided = true;

		plane->materials.AddToTail(fxMat);

		int num = 40;
		
		for (int i=0; i<num; i++)
		{
			float rnd = 0.05f;
			float x = posX;
			float y = posY;
			float z = posZ;

			CMesh *spark = new CMesh(plane, fxMat);
			spark->SetPosition(x,y,z);

			if (i%8 == 0)
				spark->SetRotation(0, frand(0,90), frand(0,90) );
			else if (i % 8 == 1)
				spark->SetRotation(0, frand(90,180),  frand(0,90) );
			else if (i % 8 == 2)
				spark->SetRotation(0, frand(180,270),  frand(0,90) );
			else if (i % 8 == 3)
				spark->SetRotation(0, frand(270,360),  frand(0,90) );
			else if (i%8 == 4)
				spark->SetRotation(0, frand(0,90),  frand(-90,0) );
			else if (i % 8 == 5)
				spark->SetRotation(0, frand(90,180),  frand(-90,0) );
			else if (i % 8 == 6)
				spark->SetRotation(0, frand(180,270),  frand(-90,0) );
			else if (i % 8 == 7)
				spark->SetRotation(0, frand(270,360), frand(-90,0) );

			spark->SetScale(1,1,frand(0.5,0.75));
			spark->MoveForward(frand(0.3,0.6));
			
			scene->Add(spark);	
			sparks.AddToTail(spark);
		}
	}

	void Frame(float time, float realtime)
	{
		if (gInput.WasKeyPressed(K_UPARROW)) mat->specular += 1.0f;
		if (gInput.WasKeyPressed(K_DOWNARROW)) mat->specular -= 1.0f;
	}

	CLight *pLight;
	CMaterial *mat;
	CArray<CMesh *> sparks;
};

class CCarDemo : public CDemo
{
public:
	CCarDemo()
	{
		scene->CreateSkybox("park");

		CMaterial *material = new CMaterial();
		CGeometry *geom = new CGeometry();
		
		pCube = (CMesh*)CModelLoader::Load("models/asv/asv.obj");
		pCube->SetScale(0.1f);
		pCube->SetRotationX(90);
		pCube->SetPosition(0,0.1f, 0);

		pGroup = new CObject3D();
		pGroup->Add(pCube);

		scene->Add(pGroup);

		CMaterial *planeMat = new CMaterial();	
		CGeometry *planeGeom = new CPlaneGeometry(10,10,1,1,planeMat);
		CMesh *plane = new CMesh(planeGeom);
		planeMat->features = EShaderFeature::LIGHT | EShaderFeature::SHADOW;
		scene->Add(plane);
		plane->SetPosition(-5,-0.5f, -5);

		scene->fog = new SFog(BLACK, 2, 20);

		scene->Add( new CDirectionalLight(Vector3(1, -0.2f, 0), WHITE, RED) );
		scene->Add( pLight = new CPointLight(Vector3(0,0,3.0f), YELLOW, 3.0f, SRGBA(255,255,200,255)) );

		// sprite
		spr = new CSprite3D( new CTexture("gfx/flare_sun.png"), 10, 10, true );
		pLight->Add(spr);

		
		CMesh *sphere = new CMesh( new CSphereGeometry(), planeMat );
		sphere->SetPosition(3,1,2);
		scene->Add(sphere);
		

		CMesh *cyl = new CMesh( new CCylinderGeometry(0), planeMat );
		cyl->SetPosition(-3,1,-2);
		scene->Add(cyl);

		pWheels[0] = pCube->GetChildByName(str("%d", 20));
		pWheels[1] = pCube->GetChildByName(str("%d", 19));
		pWheels[2] = pCube->GetChildByName(str("%d", 18));
		pWheels[3] = pCube->GetChildByName(str("%d", 17));

		for (int i=0; i<4; i++)
		{
			CMesh *w = (CMesh*)pWheels[i];
			w->Reposition();
		}

	}

	void Frame(float time, float realtime)
	{
		pLight->SetPosition(cos(realtime)*4.0f, 0 ,sin(realtime)*4.0f);	
	}

	CLight* pLight;
};




class CMapDemo : public CDemo
{
public:
	CMapDemo()
	{
		scene->CreateSkybox("clear");
		pLevel = new CLevel(scene, NULL);
		pLevel->Create(4,2,4);

		scene->fog = new SFog(SRGBA(172,202,241, 200), 5, 20);

		bbox = SBBox(Vector3(-0.25f, -0.25f, -0.25f), Vector3(0.25f, 0.25f, 0.25f));

		pCamera->SetPosition(pLevel->sizeX/2,pLevel->sizeY+3,pLevel->sizeZ/2);
	}

	void Frame(float time, float realtime)
	{
		gRenderer.AddBBox(bbox, pCamera->GetPosition());
	}

	CLight* pLight;
	SBBox bbox;
};

class CMobDemo : public CDemo
{
public:
	CMobDemo()
	{
		scene->CreateSkybox("park");

		CTexture *groundTex = new CTexture("textures/ground.png");
		CMaterial *groundMat = new CMaterial();
		groundMat->pTexture = groundTex;
		CGeometry *planeGeom = new CPlaneGeometry(8,8,1,1,groundMat);
		CMesh *plane = new CMesh(planeGeom);
		scene->Add(plane);
		plane->SetPosition(-4,-1.75f,-4);

		CTexture *tex = new CTexture("textures/player3.png");
		CMaterial *mat = new CMaterial();
		mat->features = EShaderFeature::TEXTURE | EShaderFeature::LIGHT;
		mat->pTexture = tex;
		player = new CObject3D();

		head = new CMesh( new CCubeGeometry(1,1,1));
		torso = new CMesh( new CCubeGeometry(1,1.25f,0.5f));
		larm = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
		rarm = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
		lleg = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
		rleg = new CMesh( new CCubeGeometry(0.5f,1.25f,0.5f));
		helmet = new CMesh( new CCubeGeometry(1,1,1));
		helmet->SetScale(1.1f);
		
		
		helmet->geometry->materials.AddToHead(mat);
		head->geometry->materials.AddToHead(mat);
		torso->geometry->materials.AddToHead(mat);
		larm ->geometry->materials.AddToHead(mat);
		rarm ->geometry->materials.AddToHead(mat);
		lleg ->geometry->materials.AddToHead(mat);
		rleg ->geometry->materials.AddToHead(mat);
		
		head->Add(helmet);

		for (int i=0; i<head->geometry->vertices.Size(); i++) head->geometry->vertices[i].y += 0.5f;
		for (int i=0; i<helmet->geometry->vertices.Size(); i++) helmet->geometry->vertices[i].y += 0.5f;
		for (int i=0; i<larm->geometry->vertices.Size(); i++) { larm->geometry->vertices[i].x += 0.25f; larm->geometry->vertices[i].y -= 1.25f/2.0f; }
		for (int i=0; i<rarm->geometry->vertices.Size(); i++) { rarm->geometry->vertices[i].x -= 0.25f; rarm->geometry->vertices[i].y -= 1.25f/2.0f; }
		for (int i=0; i<lleg->geometry->vertices.Size(); i++) { lleg->geometry->vertices[i].y -= 1.25f/2.0f; }
		for (int i=0; i<rleg->geometry->vertices.Size(); i++) { rleg->geometry->vertices[i].y -= 1.25f/2.0f; }

		TextureBlock(head,  2,2,2,2, 6,2,2,2, 4,2,2,2, 0,2,2,2, 2,0,2,2, 4,0,2,2);
		TextureBlock(torso, 5,5,2,3, 8,5,2,3, 7,5,1,3, 4,5,1,3, 5,4,2,1, 7,4,2,1);
		TextureBlock(larm,  11,5,1,3, 13,5,1,3, 12,5,1,3, 10,5,1,3, 11,4,1,1, 12,4,1,1);
		TextureBlock(rarm,  11,5,1,3, 13,5,1,3, 12,5,1,3, 10,5,1,3, 11,4,1,1, 12,4,1,1);
		TextureBlock(lleg,  1,5,1,3, 3,5,1,3, 2,5,1,3, 0,5,1,3, 1,4,1,1, 2,4,1,1);
		TextureBlock(rleg,  1,5,1,3, 3,5,1,3, 2,5,1,3, 0,5,1,3, 1,4,1,1, 2,4,1,1);
		TextureBlock(helmet,  10,2,2,2, 14,2,2,2, 12,2,2,2, 8,2,2,2, 10,0,2,2, 12,0,2,2);
		
		
		player->Add(head);
		player->Add(torso);
		player->Add(larm);
		player->Add(rarm);
		player->Add(lleg);
		player->Add(rleg);

		head->SetPosition(0,1.25f/2.0f,0);
		larm->SetPosition(0.5f,1.25f/2.0f,0);
		rarm->SetPosition(-0.5f,1.25f/2.0f,0);
		lleg->SetPosition(0.26f,-1.25f/2.0f,0);
		rleg->SetPosition(-0.26f,-1.25f/2.0f,0);
		
		scene->Add( pLight = new CPointLight(Vector3(-1.0f,0,-2.0f), YELLOW, 2.0f, SRGBA(255,255,200,255)) );
		scene->Add( pLight2 = new CPointLight(Vector3(+1.0f,0,-2.0f), RED, 2.0f, SRGBA(255,255,200,255)) );

		scene->ambientColor = SRGBA(100,100,250,255);

		scene->Add(player);
	}

	void Frame(float time, float realtime)
	{
		head->SetRotation(sin(realtime)*10.0f, sin(realtime)*10.0f, 0);
		lleg->SetRotation(sin(4*realtime)*30.0f,0,0);
		rleg->SetRotation(cos(4*realtime+M_PI/2.0f)*30.0f,0,0);
		larm->SetRotation(sin(2*realtime)*50.0f,0,sin(2*realtime)*30.0f);
		rarm->SetRotation(cos(2*realtime)*50.0f,0,cos(2*realtime)*30.0f);

		float speed = 4.0f;
		if (gInput.IsKeydown(K_LEFTARROW)) pLight->Move(time * -speed,0,0);
		if (gInput.IsKeydown(K_RIGHTARROW)) pLight->Move(time * +speed,0,0);
		if (gInput.IsKeydown(K_UPARROW)) pLight->Move(0,0,time * +speed);
		if (gInput.IsKeydown(K_DOWNARROW)) pLight->Move(0,0,time * -speed);

		if (gInput.WasKeyPressed(K_PGUP)) pLight2->intensity += 0.1f;
		if (gInput.WasKeyPressed(K_PGDN)) pLight2->intensity -= 0.1f;
	}
	
	CObject3D *player;	
	CLight* pLight;
	CLight* pLight2;
	CMesh *head ;
	CMesh *torso;
	CMesh *larm ;
	CMesh *rarm ;
	CMesh *lleg ;
	CMesh *rleg; 
	CMesh *helmet;
};


// Funkcja natychmiastowego zamykania aplikacji
void QuitFunc( void )
{
	if (gInput.m_bTypingMode)
	{
		gInput.EndTyping();
		return;
	}
	gFramework.EndGame();						// Konczymy gre, jesli trwa
	gEngine.Quit();				// Oznajmiamy silnikowi, ze konczymy petle
}

// Szybkie uruchomienie serwera i podlaczenie do gry
void StartFunc( void )
{
}

void EndFunc( void )
{
	gFramework.EndGame();
}



//////////////////////////////////////////////////////////////////////////
void GameFrameFunc( double fTime )
{
	gFramework.Frame( fTime );
}

//////////////////////////////////////////////////////////////////////////
CFramework::CFramework()
{
	state = GAMESTATE_MAINMENU;
	pCamera = NULL;
}

// Usuwanie obiektow gry
CFramework::~CFramework()
{

}

CDemo *pEnvDemo;
CDemo *pCarDemo;
CDemo *pMobDemo;
CDemo *pMapDemo;


CDemo *pCurrent = NULL;
CSprite2D		*pSplash;



//////////////////////////////////////////////////////////////////////////
bool CFramework::Init( void )
{
	gD3D		= gEngine.GetD3D();

	// Default bindings
	gInput.BindKey( K_ESCAPE, "quit" );
	gInput.BindKey( '`', "toggleconsole" );

	// Console panel
	gConsolePanel.Init();
	gConsolePanel.AddLine( "Game initialized" );
	gConsolePanel.AddLine("");

	// GUI
	gCursor.Init( true );
	fontDin = gFontMgr.LoadFontFromFile("fonts/Elemental End.ttf", "Elemental End", 50, false, false, true);
	
	gEffects.Init();
	gVGUI.Init();
	
	//pGame = new CEffectsGame();
	//pGame = new CHeadGame();
	pGame = new CEmptyGame();
	pGame->Init();
	pGame->PostInit();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFramework::Shutdown()
{
	SAFE_DELETE(pGame);
	gRenderer.Release();
	gCursor.Release();
	gVGUI.Release();
	
	CTileset::Release();
}

//////////////////////////////////////////////////////////////////////////
// Klatka gry
// tutaj wkladamy wszystko, co ma sie dziac podczas kazdej klatki gry, czyli
// logika serwea/clienta, input oraz rysowanie
//////////////////////////////////////////////////////////////////////////
void CFramework::Frame(double fTime)
{
	frametime = fTime;
	realtime += fTime;

	UpdateGUI();

	gEffects.Update();			// watch for effects changes
	
	pGame->Update(frametime, realtime);	
	pGame->Render(frametime, realtime);
		
	// Draw UI	
	gFrameGraph.Render();
	gConsolePanel.Render();	
	
	// Present the backbuffer contents to the display
	gD3D->Display();
}

//////////////////////////////////////////////////////////////////////////
void CFramework::UpdateGUI()
{
}

//////////////////////////////////////////////////////////////////////////
void CFramework::EndGame()
{
	state = GAMESTATE_MAINMENU;
	gInput.BindKey( K_ESCAPE, "quit" );
}

//////////////////////////////////////////////////////////////////////////
void CFramework::CreateGame( const char *pszMap )
{

}
