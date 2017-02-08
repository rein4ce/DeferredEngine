#include "platform.h"
#include "BlocksGame.h"
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
#include <Newton.h>
#include "Mesh.h"
#include "Object3D.h"
#include "keydefs.h"
#include "Input.h"
#include "Geometry.h"
#include "Array.h"

CBlocksGame::CBlocksGame()
{
	pLevel = NULL;
}

CBlocksGame::~CBlocksGame()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CBlocksGame::BlockWorldUpdate(float frametime, float realtime)
{
	Vector3 pos = pCamera->position;
	pCameraController->Update(frametime );	
	Vector3 displacement = pCamera->position - pos;

	if (displacement.Length() > 0)
	{

		Vector3 boxdir[] = { Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1) };

		SCollisionResult result;
		CArray<STriangle> tris;
		pLevel->GetCollisionFaces(pCamera->position, 1.0f, tris);

		pCamera->position = CMovement::SlideMove( 
			tris,
			pos,
			pPlayer->bbox.max,
			boxdir,
			displacement,
			result );

		if (result.collision)
			gRenderer.AddBBox(pPlayer->bbox,pCamera->GetPosition(), SRGBA(255,0,0));
			//gVGUI.AddTextMessage(300,400, RED, "%d", tris.Size());

	}

	static CMesh *pNewTile = NULL;
	static CMaterial *pHighlightMat = new CMaterial(SRGBA(255,255,255,200));
	static Vector3 selectedTile = Vector3(-1,-1,-1);
	static Vector3 newTilePos = Vector3(-1,-1,-1);

	pHighlightMat->features = 0;	
	pHighlightMat->transparent = true;
	pHighlightMat->doubleSided = true;

	selectedTile.Set(-1,-1,-1);
	newTilePos.Set(-1,-1,-1);

	// show ray
	Vector3 start = pCamera->GetPosition();
	Vector3 end = start + pCamera->forward * 5.0f;
	CArray<Vector3> tiles;
	TraverseGrid3D(start.x,start.y,start.z, end.x,end.y,end.z, 0,0,0, 1, tiles);

	gVGUI.AddTextMessage(50, 200, RED, "%d", tiles.Size());

	if (pNewTile == NULL)
	{
		pNewTile = new CMesh(CTileset::meshes[0][1]->pGeometry);
		pNewTile->geometry->materials.AddToTail(pHighlightMat);
		pNewTile->opacity = 0.75f;
		pNewTile->SetPosition(-1,-1,-1);
		pScene->Add(pNewTile);
	}

	pNewTile->opacity = sin(realtime*8)*0.125f + 0.5f;


	if (CTileset::pMaterial->pShader)
		CTileset::pMaterial->pShader->uniforms["highlightedTile"].Set(Vector3(-1,-1,-1));

	// We need geometry also for the box tile
	SMapTile box = SMapTile(1);
	CArray<STriangle> boxTris;
	box.GetCollideTriangles(0,0,0,boxTris);

	// Check the tiles along the ray and find the one we are pointing at
	for (int i=0; i<tiles.Size(); i++)
	{
		Vector3 pos = Vector3(tiles[i].x, tiles[i].y, tiles[i].z);
		SMapTile *tile = pLevel->GetTile(tiles[i].x, tiles[i].y, tiles[i].z);
		if (tile == NULL) continue;

		CArray<STriangle> tris;
		tile->GetCollideTriangles(tiles[i].x, tiles[i].y, tiles[i].z, tris);
		bool found = false;

		for (int t=0; t<tris.Size(); t++)
			if (CCollision::LineTriangle(tris[t].v[0], tris[t].v[1], tris[t].v[2], start, end))
			{
				CTileset::pMaterial->pShader->uniforms["highlightedTile"].Set(tiles[i]);

				selectedTile.Set(tiles[i].x,tiles[i].y,tiles[i].z);

				// if we are poiting at this tile, find which box face we are also poniting at
				float d = 1, minDist = 1;
				for (int j=0; j<boxTris.Size(); j++)
				{
					Vector3 v[] = {
						boxTris[j].v[0] + pos,
						boxTris[j].v[1] + pos,
						boxTris[j].v[2] + pos
					};
					if (CCollision::LineTriangle(v[0], v[1], v[2], start, end, &d))
					{
						STriangle &tri = boxTris[j];
						tri.CalculateNormal();

						if (tri.normal.Dot(pCamera->forward) > 0) continue;

						newTilePos = pos + tri.normal;
						if (!pLevel->GetTile(newTilePos.x, newTilePos.y, newTilePos.z))
						{
							newTilePos.Set(-1,-1,-1);
							pNewTile->visible = false;
						}
						else
						{
							pNewTile->visible = true;
							pNewTile->SetPosition(newTilePos);
						}	

						break;
					}
				}
				found = true;
				break;
			}

		if (found) 
		{
			break;
		}
	}


	static int currentTile = 1;
	static int oldCurrentTile = currentTile;
	
	// Remove tile
	if (gInput.WasKeyPressed(K_MOUSE1))
	{
		SMapTile *tile = NULL;
		if (tile = pLevel->GetTile(selectedTile.x, selectedTile.y, selectedTile.z))
		{
			tile->type = 0;
			pLevel->UpdateTile(selectedTile.x, selectedTile.y, selectedTile.z);
		}
	}

	// Add Tile
	if (gInput.WasKeyPressed(K_MOUSE2))
	{
		SMapTile *tile = NULL;
		if (tile = pLevel->GetTile(newTilePos.x, newTilePos.y, newTilePos.z))
		{
			tile->type = currentTile;
			pLevel->UpdateTile(newTilePos.x, newTilePos.y, newTilePos.z);
		}
	}


	// show list of tiles	
	gVGUI.AddTextMessage(gEngine.width-100, 100, WHITE, "Tiles:");
	for (int i=1; i<CTileset::tiles.Size(); i++)
	{
		gVGUI.AddTextMessage(currentTile == i ? WHITE : SRGBA(111,111,111), "%d: %s", i, CTileset::tiles[i].name.c_str());
	}

	if (gInput.WasKeyPressed(K_MWHEELUP))
	{
		currentTile--;
		if (currentTile == 0) currentTile = CTileset::tiles.Size()-1;
	}

	if (gInput.WasKeyPressed(K_MWHEELDOWN))
	{
		currentTile++;
		if (currentTile == CTileset::tiles.Size()) currentTile = 1;
	}

	if (oldCurrentTile != currentTile)
	{
		oldCurrentTile = currentTile;
		pNewTile->geometry = CTileset::tiles[currentTile].pMeshes[1]->pGeometry;
		if (pNewTile->geometry->materials.Size() == 0)
			pNewTile->geometry->materials.AddToTail(pHighlightMat);
		pNewTile->geometry->dirtyVertices = true;
	}

	gRenderer.AddBBox(pPlayer->bbox, pCamera->GetPosition());	
}

//////////////////////////////////////////////////////////////////////////
void CBlocksGame::Update( float frametime, float realtime )
{	
	BlockWorldUpdate(frametime, realtime);

	CGame::Update(frametime, realtime);
}

//////////////////////////////////////////////////////////////////////////
void CBlocksGame::Init()
{
	CGame::Init();
	CTileset::Init();

	// create level
	pLevel = new CLevel(pScene, NULL);
	pLevel->Create(4,2,4);
}

//////////////////////////////////////////////////////////////////////////
void CBlocksGame::Reset()
{
	CGame::Reset();

	SAFE_DELETE(pLevel);
}

