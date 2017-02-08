#include "platform.h"
#include "Editor.h"
#include "Entity.h"
#include "Input.h"
#include "CVar.h"
#include "Game.h"
#include "Engine.h"
#include "Tileset.h"
#include "Level.h"
#include "Scene.h"
#include "Object3D.h"
#include "Material.h"
#include "Utils.h"
#include "Array.h"
#include "Mesh.h"
#include "Renderer.h"

CEditor gEditor;

CVar cv_editorMoveSpeed("editormovespeed", "1.0", FCVAR_ARCHIVE);
CVar cv_editorRotateSpeed("editorrotatespeed", "180.0", FCVAR_ARCHIVE);

CEditor::CEditor(void)
{
	selectedEntity = NULL;
	mapPath = mapName = "Untitled.map";
	mode = EDITOR_TILES;
	CArray<string> list; 
	GetEntitiesList(list);
	selectedEntityType = list[0];
	selectedTileType = 0;
	show = true;
	pEditTile = NULL;
}

CEditor::~CEditor(void)
{
}

void CEditor::Frame(float frametime, float realtime)
{	
	// toggle editor on/off
	if (!gInput.m_bTypingMode && KeyPressed('c')) cv_fly.SetValue(1-cv_fly.GetInt());

	// if not in fly mode, editor is off
	show = cv_fly.GetInt() == 1;

	ToggleEditTile(false);

	// shortcuts
	if (KeyPressed(K_F6)) Save();
	if (KeyPressed(K_F7)) Load(mapPath);


	if (!show)
	{
		gInput.SetCenterCursor(true);
		return;
	}

	// swtich modes
	if (KeyPressed(K_F1)) mode = EDITOR_TILES;
	if (KeyPressed(K_F2)) mode = EDITOR_ENTITIES;
	if (KeyPressed(K_F3)) mode = EDITOR_ENVIRONMENT;


	// Process input for different modes
	if (mode == EDITOR_TILES) ProcessTilesInput(); else
	if (mode == EDITOR_ENTITIES) ProcessEntitiesInput();

	// Pointer lock - look around when right click
	if (KeyPressed(K_MOUSE2)) gInput.SetCenterCursor(true);
	if (KeyReleased(K_MOUSE2)) gInput.SetCenterCursor(false);
}

void CEditor::Save()
{
	Save(mapPath);
}

void CEditor::Save( string filename )
{
	pGame->Save(filename);
}

void CEditor::Init()
{
	// Edit tile
	pEditTile = new CObject3D();
	pEditTile->visible = false;
	pEditTile->opacity = 0.5f;
	pEditTile->castShadow = false;
	pGame->pScene->Add(pEditTile);
}

void CEditor::SaveAs()
{
	OPENFILENAME desc;
	char file[256];
	ZeroMemory(&desc, sizeof(desc));
	desc.hwndOwner = gEngine.GetHWND();
	desc.lStructSize = sizeof(desc);
	desc.lpstrFile = file;
	desc.lpstrFile[0] = '\0';
	desc.nMaxFile = sizeof(file);
	desc.lpstrInitialDir = NULL;	

	if (GetOpenFileName(&desc))
	{
		Save(file);
	}
}

void CEditor::Load(string path)
{
	pGame->Load(path);
	mapPath = path;
	mapName = GetPathFileName(path);
	Reset();
}

void CEditor::Load()
{	
	OPENFILENAME desc;
	char file[256];
	ZeroMemory(&desc, sizeof(desc));
	desc.hwndOwner = gEngine.GetHWND();
	desc.lStructSize = sizeof(desc);
	desc.lpstrFile = file;
	desc.lpstrFile[0] = '\0';
	desc.nMaxFile = sizeof(file);
	desc.lpstrInitialDir = NULL;
	desc.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&desc))
	{
		string filename = file;
		Load(filename);
	}	
}

void CEditor::Reset()
{
	selectedEntity = NULL;
}

void CEditor::GetEntitiesList(CArray<string> &list)
{
	string entityNames[] = 
	{
		"SpawnPoint",
		"Turret",
		"Generator",
		"Waypoint",
		"Enemy",
		"Spawner",
		"Particle Emitter",
		"Model",
		"SemiTruck",
		"Light"
	};

	list.Purge();
	int num = ArraySize(entityNames);
	for (int i=0; i<num; i++) list.Add(entityNames[i]);
}

//////////////////////////////////////////////////////////////////////////
void CEditor::ProcessEntitiesInput()
{
	static bool reverse = false;
	static int lastKey;

	float frametime = gEngine.frametime;
	float realtime = gEngine.realtime;

	ToggleEditTile(false);
	
	// choose next entity
	if (KeyPressed(K_MWHEELDOWN)) 
	{
		int i;
		CArray<string> list; 
		GetEntitiesList(list);
		for (i=0; i<list.Size(); i++)
			if (selectedEntityType == list[i]) break;

		if (i == list.Size()-1) i = 0; else i++;
		selectedEntityType = list[i];
	}

	// choose previous entity
	if (KeyPressed(K_MWHEELUP))
	{
		int i;
		CArray<string> list; 
		GetEntitiesList(list);
		for (i=0; i<list.Size(); i++)
			if (selectedEntityType == list[i]) break;

		if (i == 0) i = list.Size()-1; else i--;
		selectedEntityType = list[i];
	}

	// selected entity manipulation
	if (selectedEntity)
	{
		int keys[] = 
		{
			'x', 'y', 'z'
		};

		for (int i=0; i<ArraySize(keys); i++)
		{
			if (!gInput.m_bTypingMode && KeyPressed(keys[i]))
			{				
				lastKey = keys[i];
			}

			if (!gInput.m_bTypingMode && KeyReleased(keys[i]))
			{
				if (lastKey == keys[i]) reverse = !reverse;
			}
		}

		float moveSpeed = cv_editorMoveSpeed.GetFloat() * (reverse ? -1 : 1);
		float rotateSpeed = cv_editorRotateSpeed.GetFloat() * (reverse ? -1 : 1);

		// move it around
		if (Keydown(K_LEFTARROW)) selectedEntity->Move(-frametime * moveSpeed,0,0);
		if (Keydown(K_RIGHTARROW)) selectedEntity->Move(+frametime * moveSpeed,0,0);
		if (Keydown(K_UPARROW)) selectedEntity->Move(0,0,+frametime * moveSpeed);
		if (Keydown(K_DOWNARROW)) selectedEntity->Move(0,0,-frametime * moveSpeed);
		if (Keydown(K_PGUP)) selectedEntity->Move(0,+frametime * moveSpeed,0);
		if (Keydown(K_PGDN)) selectedEntity->Move(0,-frametime * moveSpeed,0);

		// rotate
		if (Keydown('x')) selectedEntity->Rotate(-frametime * rotateSpeed,0,0);
		if (Keydown('y')) selectedEntity->Rotate(0,+frametime * rotateSpeed,0);
		if (Keydown('z')) selectedEntity->Rotate(0,0,+frametime * rotateSpeed);

		// delete
		if (Keydown(K_DEL))
		{
			pGame->RemoveEntity(selectedEntity);
			selectedEntity = NULL;
		}
	}



	// if we are looking around
	if (Keydown(K_MOUSE2))
	{
		// cast ray and check if we are pointing at another entity
		Vector3 start = pGame->pCamera->GetWorldPosition();
		Vector3 end = start + pGame->pCamera->forward * 1000.0f;
		bool hit = false;
		SRayCastResult rc;
		if (hit = pGame->CastRay(start, end, RC_OBJECTS | RC_TERRAIN | RC_VOXEL, OUT rc))
		{
			// if it's an object, highlight it and allow selection
			if (rc.type == RC_OBJECTS)
			{				
				if (rc.object != selectedEntity)
					gRenderer.Addsphere(rc.object->GetWorldPosition(), 1, GREEN);

				// Select entity under the cursor
				if (KeyPressed(K_MOUSE1))
				{
					selectedEntity = (CEntity*)rc.object;
					Debug("Selected entity: %s", selectedEntity->GetType().c_str());
				}
			}
			else
			{
				// Spawn entity
				if (KeyPressed(K_MOUSE1))
				{
					CEntity *ent = CEntity::Create(selectedEntityType);
					//ent->pTarget = pCar;

					if (ent->GetType() == "Waypoint" || ent->GetType() == "Enemy")
						ent->Spawn(rc.hitPos+Vector3(0,1,0));
					else
						ent->Spawn(rc.hitPos);

					selectedEntity = ent;
					pGame->AddEntity(ent);
				}
			}
		}

		// Draw entity selection
		if (selectedEntity)
		{
			gRenderer.Addsphere(selectedEntity->GetWorldPosition(), 1, RED);
		}
	}
	

	if (selectedEntity)
		selectedEntity->UpdateMatrix();
}

//////////////////////////////////////////////////////////////////////////
void CEditor::ToggleEditTile(bool visible)
{
	pEditTile->visible = visible;
	if (pEditTile->children.Size() > 0)
		pEditTile->children[0]->visible = visible;
}

//////////////////////////////////////////////////////////////////////////
void CEditor::ProcessTilesInput()
{
	bool hit = false;
	Vector3 tileHitPos;
	SMapTile *tileHit;
	Vector3 start = pGame->pCamera->GetWorldPosition();
	Vector3 end = start + pGame->pCamera->forward * 1000.0f;
	Vector3 pos;

	// Editor variables
	static int tileRotation = 0;	
	static int lastSelectedTile = -1;	

	pLevel->UnHighlightTile();


	// choose next tile
	if (KeyPressed(K_MWHEELDOWN)) 
	{		
		selectedTileType++;
		if (selectedTileType >= CTileset::tiles.Size()) selectedTileType = 0;
	}

	// choose previous tile
	if (KeyPressed(K_MWHEELUP))
	{
		selectedTileType--;
		if (selectedTileType < 0) selectedTileType = CTileset::tiles.Size()-1;
	}

	ToggleEditTile(true);

	// Change editable tile if visible
	if (lastSelectedTile != selectedTileType)
	{			
		STileType *type = &CTileset::tiles[selectedTileType];			
		pEditTile->Empty();
		//type = &CTileset::tiles[selectedTileType];	

		if (type->pMeshes)
		{
			CObject3D *mesh = type->pMeshes[0]->pMesh->Copy();
			pEditTile->Add(mesh);
			mesh->SetPosition(-0.5f, 0, -0.5f);
		}
		ToggleEditTile(false);
		lastSelectedTile = selectedTileType;
	}

	// Tile rotation on 'Q'
	if (!gInput.m_bTypingMode && KeyPressed('q')) tileRotation++;
	if (tileRotation == 4) tileRotation = 0;


	// Remove tile 
	if (KeyPressed(K_MOUSE1))
	{
		if (hit = pLevel->CastRay(start, end, OUT tileHitPos, OUT &tileHit, OUT &pos))
		{
			tileHit->type = 0;
			pLevel->UpdateTile(tileHitPos.x, tileHitPos.y, tileHitPos.z);			
		}
	}


	// Find tile side we are pointing at
	ETileSide::Type side;
	hit = pLevel->CastRaySide(start, end, OUT tileHitPos, OUT &tileHit, OUT side, OUT &pos);
	if (hit)
	{	
		Vector3 disp;							// destination tile displacement
		switch ( side )							// determine the position of the neighbour tile
		{
			case ETileSide::Left:		disp.x--; break;
			case ETileSide::Right:		disp.x++; break;
			case ETileSide::Front:		disp.z--; break;
			case ETileSide::Back:		disp.z++; break;
			case ETileSide::Top:		disp.y++; break;
			case ETileSide::Bottom:		disp.y--; break;
		}

		tileHitPos += disp;

		SMapTile *tile = pLevel->GetTile(tileHitPos.x, tileHitPos.y, tileHitPos.z);
		if (tile) 
		{
			// Position edit tile
			Vector3 position = tileHitPos + pLevel->offset + Vector3(0.5f, 0, 0.5f);
			pEditTile->SetPosition(position);
			ToggleEditTile(true);

			int finalTileRotation = tileRotation;
			
			// Check relative angle between camera and destination tile
			switch ( side )							// determine the position of the neighbour tile
			{
			case ETileSide::Left:		finalTileRotation += 1; break;
			case ETileSide::Right:		finalTileRotation += 3; break;
			case ETileSide::Front:		finalTileRotation += 0; break;
			case ETileSide::Back:		finalTileRotation += 2; break;				
			}

			if (finalTileRotation > 3) finalTileRotation -= 4;

			pEditTile->SetRotationY(finalTileRotation*90.0f);			
					
			// If pressed Right Mouse Button - edit tile
			if (!gInput.m_bTypingMode && KeyPressed('e'))
			{
				tile->type = selectedTileType;
				tile->rotation = finalTileRotation;
				pLevel->UpdateTile(tileHitPos.x, tileHitPos.y, tileHitPos.z);
			}			
		}	
	}
	else
	{
		ToggleEditTile(false);
	}
}