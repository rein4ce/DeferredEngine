// VoidGame.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")

#include "platform.h"
#include <windows.h>
#include "platform.h"
#include "Engine.h"
#include "Framework.h"
#include "ConsolePanel.h"
#include "CVar.h"
#include <D3Dcommon.h>
#include <D3D10.h>
#include "Object3D.h"
#include "Shader.h"
#include "Console.h"
#include "Font.h"
#include "Direct3D.h"

CVar cv_server	( "server", "1" );

//////////////////////////////////////////////////////////////////////////
void ConsoleError(char *pText)
{
	gConsolePanel.AddLine( SRGB(255,70,70), pText );
}

void ConsolePrint(char *pText)
{
	gConsolePanel.AddLine( SRGB(255,255,255), pText );
}

void ConsoleOutputFunc(char *pText)
{
	gConsolePanel.AddLine( SRGB(255,70,70), pText );
}


ID3D10Device*					g_pDevice;


//////////////////////////////////////////////////////////////////////////
// Initialize the modules before creating the window
//////////////////////////////////////////////////////////////////////////
void InitModules()
{
	gConsole.Init( CCommandMgr::GetList() );
}

//////////////////////////////////////////////////////////////////////////
// Initialize the modules after creating the window
//////////////////////////////////////////////////////////////////////////
void PostInitModules()
{
	gConsole.SetOutputFunc( ConsoleOutputFunc );
	gFontMgr.Init();
	g_pDevice = gEngine.GetD3D()->GetDevice();
}

//////////////////////////////////////////////////////////////////////////
// Release modules
//////////////////////////////////////////////////////////////////////////
void ReleaseModules()
{
	gFontMgr.Release();
}

//////////////////////////////////////////////////////////////////////////
// Load config.cfg
//////////////////////////////////////////////////////////////////////////
void LoadConfigFile()
{
	gConsole.AddCommand( "exec config/config.cfg" );
	gConsole.Execute();
}

//////////////////////////////////////////////////////////////////////////
// WinMain
//////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR   lpCmdLine,
					 int       nCmdShow)
{
	InitModules();
	LoadConfigFile();

	srand(time(NULL));
	
	gEngine.CreateEngineWindow( "VoidGame" );
	gEngine.SetFrameFunc( GameFrameFunc );
	PostInitModules();

	if ( !gFramework.Init() ) 
	{
		ReleaseModules();
		return 1;
	}

	gEngine.Loop();
	gFramework.EndGame();
	gEngine.Shutdown();
	gConsole.WriteVariables();

	gFramework.Shutdown();
	
	ReleaseModules();
	return 0;
}