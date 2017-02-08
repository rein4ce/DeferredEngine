#include "platform.h"
#include "GameOverMenu.h"
#include "Font.h"
#include "VGUI.h"
#include "Engine.h"
#include "Framework.h"
#include "Game.h"
#include "Input.h"
#include "Sprite.h"
#include "Renderer.h"

extern VoidFont				fontDin;

CGameOverMenu::CGameOverMenu(void)
{
	opacity = 0;
	showing = false;

	background = new CSprite2D();
	background->position = Vector2(0,0);
	background->size = Vector2(gEngine.width, gEngine.height);
	background->color = SRGBA(0,0,0,0);
	background->z = 100;
	background->visible = false;
	gRenderer.AddSprite(background);
}

CGameOverMenu::~CGameOverMenu(void)
{
}

void CGameOverMenu::Update( float frametime )
{
	static float x = 0;
	if (!background->visible) return;

	if (showing && opacity < 1)
	{
		background->visible = true;
		opacity += frametime;
		if (opacity > 1) opacity = 1;
	}
	
	if (!showing && opacity > 0)
	{
		opacity -= frametime;
		if (opacity < 0) 
		{
			opacity = 0;
			background->visible = false;
			x = 0;
		}
	}

	if (!showing && opacity == 0) x = 0; 

	if (opacity >= 1)
	{
		static float bgalpha = 0;
		bgalpha += frametime;
		Clamp(bgalpha, 0, 1);
		background->color = SRGBA(0,0,0,255.0f * bgalpha);
	}	

	float w,h;
	

	if (pGame->realtime > showTime + 3 && x < 200)
	{
		x += frametime * 400.0f;
	}

	gFontMgr.GetTextSize(fontDin, "MISSION", OUT w, OUT h);
	gVGUI.AddTextMessageEx(gEngine.width/2 - w/2, gEngine.height/2 - 40 - (1.0f-opacity) * (gEngine.height/2 - 40) - x, WHITE, fontDin, true, opacity * 255.0f, "MISSION" );
	gFontMgr.GetTextSize(fontDin, "ACCOMPLISHED", OUT w, OUT h);
	gVGUI.AddTextMessageEx(gEngine.width/2 - w/2, gEngine.height/2 + 40 + (1.0f-opacity) * (gEngine.height/2 + 40) - x, WHITE, fontDin, true, opacity * 255.0f, "ACCOMPLISHED" );

	if (x >= 200)
	{
		char *s = "Start again? [Y] to continue or [ESC] to quit";
		gFontMgr.GetTextSize(0, s, OUT w, OUT h);
		PrintText(gEngine.width/2 - w/2, gEngine.height/2 - h/2, s);

		if (KeyPressed('y'))
		{
			pGame->Load("level.map");
			showing = false;
			x = 0;
		}
	}
}

void CGameOverMenu::Show()
{
	showing = true;
	showTime = pGame->realtime;
	background->visible = true;
}

void CGameOverMenu::Hide()
{
	showing = false;
}