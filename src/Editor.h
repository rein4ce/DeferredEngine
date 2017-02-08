#pragma once
#include "platform.h"
#include "Array.h"

class CObject3D;
class CEntity;

namespace EEditorMode { enum Type
{
	Tiles,
	Entities
}; };

enum
{
	EDITOR_TILES,
	EDITOR_ENTITIES,
	EDITOR_ENVIRONMENT
};


class CEditor
{
public:
	CEditor(void);
	~CEditor(void);

	void Reset();
	void Frame(float frametime, float realtime);
	void Init();

	void ProcessEntitiesInput();
	void ProcessTilesInput();

	void Save();
	void SaveAs();
	void Save(string filename);
	void Load();
	void Load(string path);

	void ToggleEditTile(bool visible);
	void GetEntitiesList(CArray<string> &list);

public:
	CEntity			*selectedEntity;
	CObject3D		*pEditTile;

	string			mapName;
	string			mapPath;

	int				mode;
	bool			show;
	int				selectedTileType;
	string			selectedEntityType;
};

extern CEditor gEditor;