#include "platform.h"
#include "VGUI.h"
#include "Font.h"
#include "Renderer.h"
#include "GeometryRenderer.h"
#include "Engine.h"
#include "VGUIControls.h"
#include "Input.h"
#include "Config.h"
#include "Editor.h"
#include "Entity.h"
#include "Properties.h"
#include "ModelLoader.h"
#include "Tileset.h"

using namespace GUI;

CVGUI			gVGUI;

namespace GUI 
{
	enum
	{
		TAB_TILES,
		TAB_ENTITIES,
		TAB_ENVIRONMENT
	};

	enum 
	{
		BTN_SAVE,
		BTN_SAVEAS,
		BTN_OPEN,
		BTN_MOVE,
		BTN_ROTATE,
		BTN_SCALE,
		BTN_CREATE,
		BTN_DELETE,
		BTN_NUM
	};

	Panel *MainPanel;						// fullscreen panel
	Panel *SidePanel;						// right side panel
	Panel *EntityPropertiesPanel;			// properties tab
	Panel *TilesPanel;
	Panel *EnvironmentPanel;
	Panel *EntitiesListPanel;
	Panel *TilesListPanel;
	Panel *SelectedEntityProperties;
	Panel *LevelTitle;
	Panel *Tabs[3];
	Panel *IconsBar;
	Panel *Icons[BTN_NUM];

	void OnTilesTabClick(Panel *panel, int x, int y)
	{
		gEditor.mode = EDITOR_TILES;		
	}

	void OnEntitiesTabClick(Panel *panel, int x, int y)
	{
		gEditor.mode = EDITOR_ENTITIES;		
	}

	void OnEnvironmentTabClick(Panel *panel, int x, int y)
	{
		gEditor.mode = EDITOR_ENVIRONMENT;		
	}

	void OnSaveClick(Panel *panel, int x, int y)
	{
		gEditor.Save();
	}

	void OnSaveAsClick(Panel *panel, int x, int y)
	{
		gEditor.SaveAs();
	}

	void OnLoadClick(Panel *panel, int x, int y)
	{
		gEditor.Load();
	}

	void OnEntityTypeClick(Panel *panel, int x, int y)
	{		
		gEditor.selectedEntityType = panel->text;
	}

	void OnTileTypeClick(Panel *panel, int x, int y)
	{		
		for (int i=0; i<CTileset::tiles.Size(); i++)
			if (CTileset::tiles[i].name == panel->text)
			{
				gEditor.selectedTileType = i;
				return;
			}
	}
}


//////////////////////////////////////////////////////////////////////////
CVGUI::CVGUI()
{
	lastX = 0;
	lastY = 0;	
}

//////////////////////////////////////////////////////////////////////////
CVGUI::~CVGUI()
{
	for (int i=0; i<elements.Size(); i++)
		elements[i]->Remove();
	elements.PurgeAndDeleteElements();
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::Release()
{

}

//////////////////////////////////////////////////////////////////////////
void CVGUI::Init()
{
	LoadStyles();

	MainPanel = new Panel();	
	MainPanel->SetClass("main-panel");	
	
	// side panel
	SidePanel = new Panel();
	SidePanel->SetClass("side-panel");

	// map title
	LevelTitle = new Panel();
	LevelTitle->SetClass("level-title");
	LevelTitle->pBind = new SPropertyDefine<string>("Map name", &gEditor.mapName);
	SidePanel->Add(LevelTitle);

	// tabs
	Tabs[TAB_TILES] = new Panel();
	Tabs[TAB_TILES]->SetClass("tab tab-tiles");
	Tabs[TAB_TILES]->onClick = OnTilesTabClick;

	Tabs[TAB_ENTITIES] = new Panel();
	Tabs[TAB_ENTITIES]->SetClass("tab tab-entities");
	Tabs[TAB_ENTITIES]->onClick = OnEntitiesTabClick;

	Tabs[TAB_ENVIRONMENT] = new Panel();
	Tabs[TAB_ENVIRONMENT]->SetClass("tab tab-environment");
	Tabs[TAB_ENVIRONMENT]->onClick = OnEnvironmentTabClick;

	SidePanel->Add(Tabs[0]);
	SidePanel->Add(Tabs[1]);
	SidePanel->Add(Tabs[2]);	

	// icons bar
	IconsBar = new Panel();
	IconsBar->SetClass("icons");
	SidePanel->Add(IconsBar);

	// icons
	for (int i=0; i<BTN_NUM; i++)
	{
		Icons[i] = new Panel();
		Icons[i]->SetClass(str("icon icon%d", i));
		Icons[i]->style.y = 3+i * 34;
		IconsBar->Add(Icons[i]);
	}

	// icon events
	Icons[BTN_SAVE]->onClick = OnSaveClick;
	Icons[BTN_SAVEAS]->onClick = OnSaveAsClick;
	Icons[BTN_OPEN]->onClick = OnLoadClick;
	
	// property panels
	EntityPropertiesPanel = new Panel();
	EntityPropertiesPanel->SetClass("prop-panel");
	EntityPropertiesPanel->visible = false;

	TilesPanel = new Panel();
	TilesPanel->SetClass("prop-panel");
	TilesPanel->visible = true;

	EnvironmentPanel = new Panel();
	EnvironmentPanel->SetClass("prop-panel");
	EnvironmentPanel->visible = false;


	// entities list panel
	EntitiesListPanel = new Panel();
	EntitiesListPanel->SetClass("entities-list");
	EntityPropertiesPanel->Add(EntitiesListPanel);

	// add entity types to the list
	CArray<string> entList;
	gEditor.GetEntitiesList(entList);
	int maxPerColumn = 7;
	for (int i=0; i<entList.Size(); i++)
	{
		Panel *btn = new Panel();
		btn->SetClass("list-item");
		btn->style.x = i >= maxPerColumn ? 100 : 0;
		btn->style.y = (i % maxPerColumn) * btn->style.height;
		btn->text = entList[i];
		btn->onClick = OnEntityTypeClick;
		EntitiesListPanel->Add(btn);
	}


	// tiles list panel
	TilesListPanel = new Panel();
	TilesListPanel->SetClass("tiles-list");
	TilesPanel->Add(TilesListPanel);

	// add tile types to the list
	maxPerColumn = 10;
	for (int i=0; i<CTileset::tiles.Size(); i++)
	{
		Panel *btn = new Panel();
		btn->SetClass("list-item");
		btn->style.x = i >= maxPerColumn ? 100 : 0;
		btn->style.y = (i % maxPerColumn) * btn->style.height;
		btn->text = CTileset::tiles[i].name;
		btn->onClick = OnTileTypeClick;
		TilesListPanel->Add(btn);
	}


	SelectedEntityProperties = new Panel();
	SelectedEntityProperties->SetClass("entity-properties");
	SelectedEntityProperties->visible = false;
	EntityPropertiesPanel->Add(SelectedEntityProperties);

	
	elements.Add(MainPanel);
	MainPanel->Add(SidePanel);
	SidePanel->Add(EntityPropertiesPanel);	
	SidePanel->Add(TilesPanel);	
	SidePanel->Add(EnvironmentPanel);	

	gEditor.mode = EDITOR_TILES;

	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::Frame(float frametime, float realtime)
{
	static CEntity *oldSelectedEntity = NULL;

	// toggle editor visibility
	MainPanel->visible = gEditor.show;
	
	// create entity properties panel if entity selected
	if (gEditor.selectedEntity != oldSelectedEntity)
	{
		CreateEntityPropertiesPanel();
		oldSelectedEntity = gEditor.selectedEntity;
	}
	
	// when we clicked somewhere, reset focus
	if (gInput.WasKeyPressed(K_MOUSE1))
		for (int i=0; i<elements.Size(); i++)
			elements[i]->SetFocus(false, true);

	// adjust tabs style for editor
	TilesPanel->visible = 
	EntityPropertiesPanel->visible = 
	EnvironmentPanel->visible = false;

	Tabs[TAB_TILES]->style.background =
	Tabs[TAB_ENTITIES]->style.background =
	Tabs[TAB_ENVIRONMENT]->style.background = SRGBA(0,0,0,50);

	switch (gEditor.mode)
	{
	case EDITOR_TILES:
		TilesPanel->visible = true;
		Tabs[TAB_TILES]->style.background = SRGBA(0,0,0,222);		
		break;

	case EDITOR_ENTITIES:
		EntityPropertiesPanel->visible = true;
		Tabs[TAB_ENTITIES]->style.background = SRGBA(0,0,0,222);		
		break;

	case EDITOR_ENVIRONMENT:
		EnvironmentPanel->visible = true;
		Tabs[TAB_ENVIRONMENT]->style.background = SRGBA(0,0,0,222);		
		break;
	}

	static string oldSelectedEntityType = gEditor.selectedEntityType;
	static int oldSelectedTileType = gEditor.selectedTileType;

	// entity types list
	for (int i=0; i<EntitiesListPanel->children.Size(); i++)
	{
		if (oldSelectedEntityType == EntitiesListPanel->children[i]->text && oldSelectedEntityType != gEditor.selectedEntityType)
		{
			EntitiesListPanel->children[i]->ApplyClassStyles();			
			oldSelectedEntityType = gEditor.selectedEntityType;
		}
		
		if (gEditor.selectedEntityType == EntitiesListPanel->children[i]->text)
		{
			EntitiesListPanel->children[i]->style.background = SRGBA(0,0,0,200);
			EntitiesListPanel->children[i]->style.color = WHITE;
		}
	}

	// tiles types list
	for (int i=0; i<TilesListPanel->children.Size(); i++)
	{
		if (oldSelectedTileType == i && gEditor.selectedTileType != oldSelectedTileType)
		{
			TilesListPanel->children[i]->ApplyClassStyles();
			oldSelectedTileType = gEditor.selectedTileType;
		}

		if (CTileset::tiles[gEditor.selectedTileType].name == TilesListPanel->children[i]->text)
		{
			TilesListPanel->children[i]->style.background = SRGBA(0,0,0,200);
			TilesListPanel->children[i]->style.color = WHITE;
		}
	}

	// reload styles
	if (KeyPressed(K_F5))
		LoadStyles();
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::LoadStyles()
{
	ConfigFile config = styles = CConfig::Read("gui.txt");

	GUI::Panel::defaultStyle	= GUI::Panel::ReadStyle(config, "Panel");
	GUI::Input::defaultStyle	= GUI::Panel::ReadStyle(config, "Input");
	GUI::CheckBox::defaultStyle	= GUI::Panel::ReadStyle(config, "CheckBox");

	// refresh style of every created element
	for (int i=0; i<elements.Size(); i++)	
	{
		// apply default style
		elements[i]->ApplyStyle(config, elements[i]->GetType());	

		// apply class styles
		elements[i]->ApplyClassStyles();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::Render()
{	
	int oldCursor = cursor;
	
	ShowCursor(!gInput.m_bCenterCursor || gEditor.show);

	gRenderer.pGeometryRenderer->ortho = true;
	gRenderer.Clear(false, true);
	gRenderer.SetStencilState(true, true);

	ResetZindex();
	for (int i=0; i<elements.Size(); i++)
		elements[i]->Draw();

	gRenderer.pGeometryRenderer->Render();

	DrawTextMessages(); 

	if (cursor != CURSOR_NONE && oldCursor != cursor)
	{
		switch (cursor)
		{
		case CURSOR_NORMAL:	SetCursor(LoadCursor(NULL, IDC_ARROW)); break;
		case CURSOR_HAND:	SetCursor(LoadCursor(NULL, IDC_HAND)); break;
		case CURSOR_CROSS:	SetCursor(LoadCursor(NULL, IDC_CROSS)); break;
		case CURSOR_BEAM:	SetCursor(LoadCursor(NULL, IDC_IBEAM)); break;
		}		
	}
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::AddTextMessage( SRGB color, const char *text, ... )
{	
	lastY += 12;
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddTextMessageEx( lastX, lastY, color, 0, 1, 255, pBuffer );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::AddTextMessage( const char *text, ... )
{	
	lastY += 12;
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddTextMessageEx( lastX, lastY, WHITE, 0, 1, 255, pBuffer );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::AddTextMessage( int x, int y, SRGB color, const char *text, ... )
{
	lastX = x;
	lastY = y;
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddTextMessageEx( x, y, color, 0, 1, 255, pBuffer );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::AddTextMessage( int x, int y,const char *text, ... )
{
	lastX = x;
	lastY = y;
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddTextMessageEx( x, y, WHITE, 0, 1, 255, pBuffer );
	va_end(args);
}

void CVGUI::AddTextMessageEx( int x, int y, SRGB color, VoidFont font, bool shadow, byte opacity, const char *text, ... )
{
	lastX = x;
	lastY = y;
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	SStoredText s;
	s.x = x;
	s.y = y;
	s.color = color;
	s.font = font;
	s.shadow = shadow;
	s.opacity = opacity;
	s.text.assign(pBuffer);
	m_StoredTextMessages.AddToHead( s );
	va_end(args);
}


//////////////////////////////////////////////////////////////////////////
void CVGUI::DrawTextMessages( void )
{
	static const int CellSize = 12;

	while ( m_StoredTextMessages.Size() > 0 )
	{
		SStoredText *s = &m_StoredTextMessages[0];
		SRGBA c = SRGBA( s->color.r, s->color.g, s->color.b, s->opacity );
		if ( s->shadow )
			gFontMgr.DrawString( s->x+1, s->y+1, s->font, SRGBA(0,0,0,200), s->text.c_str() );
		gFontMgr.DrawString( s->x, s->y, s->font, c, s->text.c_str() );
		m_StoredTextMessages.RemoveAt(0);
	}
}

void EnterEntityFileName(Panel *panel)
{
	string path = panel->pBind->GetValue();
	Debug("Filename: %s", path.c_str());

	CEntity *ent = gEditor.selectedEntity;
	ent->Empty();

	ent->Add(CModelLoader::Get(path));
}

//////////////////////////////////////////////////////////////////////////
void CVGUI::CreateEntityPropertiesPanel()
{
	SelectedEntityProperties->Empty();
	

	// if no entity is selected, hide the panel
	if (!gEditor.selectedEntity) 
	{
		SelectedEntityProperties->visible = false;
		return;
	}

	Panel *container = new Panel();
	container->style.y = 50;	
	SelectedEntityProperties->Add(container);
	SelectedEntityProperties->visible = true;

	if (!gEditor.selectedEntity) return;

	CEntity *ent = gEditor.selectedEntity;

	int i;

	// position, rotattion, scale
	for (int i=0; i<3; i++)
	{
		string text;

		switch(i)
		{
		case 0: text = "Pos"; break;
		case 1: text = "Rot"; break;
		case 2: text = "Scl"; break;
		}

		Panel *label = new Panel();
		label->SetClass("prop-label");
		label->style.y = 23 * i;
		label->text = text;

		for (int j=0; j<3; j++)
		{
			Panel *input = new Input();
			input->SetClass("prop-number prop-tiny");
			input->style.x = 30 + j * 54;
			input->style.y = label->style.y;
			input->pBind = ent->properties[i*3+j];
			container->Add(input);
		}

		container->Add(label);		
	}

	for (i=9; i<ent->properties.Size(); i++)
	{
		SProperty *prop = ent->properties[i];

		Panel *label = new Panel();
		label->SetClass("prop-label");
		label->style.y = 100 + (i-9) * 23;
		label->text = prop->name;

		Panel *input = NULL;

		switch (prop->GetType())
		{
		case PROPERTY_CHECKBOX:
			input = new CheckBox();
			input->SetClass("prop-checkbox");
			input->style.x = 5;
			label->style.x += 30;
			break;
	
		case PROPERTY_FILE:
			input = new FileInput();
			input->SetClass("prop-file-input");
			input->style.x = 30;		
			input->onEnter = EnterEntityFileName;
			break;

		case PROPERTY_NUMBER:
			input = new Input();
			input->SetClass("prop-number");
			input->style.x = 30;	
			break;

		default:
			input = new Input();
			input->SetClass("prop-input");
			input->style.x = 30;			
		}
		
		input->style.y = label->style.y;				
		input->pBind = prop;
		

		container->Add(label);
		container->Add(input);
	}
}