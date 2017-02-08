#include "platform.h"
#include "Engine.h"
#include "Font.h"
#include "Input.h"
#include "Console.h"
#include "CVar.h"
#include "VGUI.h"
#include "Renderer.h"
#include "ResourceMgr.h"
#include "ModelLoader.h"
#include "FrameGraph.h"
#include "3rdparty/devil/IL/il.h"

CVar		cv_Developer		( "developer",		"1" );
CVar		cv_WindowWidth		( "w_width",		"860",	FCVAR_ARCHIVE );			// 860
CVar		cv_WindowHeight		( "w_height",		"480",	FCVAR_ARCHIVE );			// 480
CVar		cv_Fullscreen		( "w_fullscreen",	"0",	FCVAR_ARCHIVE );
CVar		cv_BPP				( "w_bpp",			"32",	FCVAR_ARCHIVE );
CVar		fps_max				( "fps_max",		"0",	FCVAR_ARCHIVE );

SFramegraph framegraph;
CEngine		gEngine;
CFrameGraph	gFrameGraph;
Direct3D	*gD3D;

//////////////////////////////////////////////////////////////////////////
// Mapowanie klawiszy
//////////////////////////////////////////////////////////////////////////
static BYTE s_pScanToEngine[128] = 
{ 
	//  0           1       2       3       4       5       6       7 
	//  8           9       A       B       C       D       E       F 
	0  ,    K_ESCAPE, '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'' ,  '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,' ',   K_CAPSLOCK  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0  , K_HOME, 
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW,K_KP_PLUS,K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,  K_F11, 
	K_F12,	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 

//////////////////////////////////////////////////////////////////////////
// Callbacki dla eventow okna Win32
//////////////////////////////////////////////////////////////////////////
CGameMessageHandler g_GameMessageHandlers[] = 
{
	{ 0xFFFF /*m_uiMouseWheel*/,		&CEngine::HandleMsg_UIMouseWheel },
	{ WM_MOUSEWHEEL,					&CEngine::HandleMsg_MouseWheel },
	//{ WM_MOVE,							&CEngine::HandleMsg_WMMove },
	//{ WM_ACTIVATEAPP,					&CEngine::HandleMsg_ActivateApp },
	{ WM_KEYDOWN,						&CEngine::HandleMsg_KeyDown },
	{ WM_SYSKEYDOWN,					&CEngine::HandleMsg_KeyDown },
	{ WM_SYSKEYUP,						&CEngine::HandleMsg_KeyUp },
	{ WM_KEYUP,							&CEngine::HandleMsg_KeyUp },
	{ WM_MOUSEMOVE,						&CEngine::HandleMsg_MouseMove },

	{ WM_LBUTTONDOWN,					&CEngine::HandleMsg_ButtonDown },
	{ WM_RBUTTONDOWN,					&CEngine::HandleMsg_ButtonDown },
	{ WM_MBUTTONDOWN,					&CEngine::HandleMsg_ButtonDown },

	{ WM_LBUTTONUP,						&CEngine::HandleMsg_ButtonUp },
	{ WM_RBUTTONUP,						&CEngine::HandleMsg_ButtonUp },
	{ WM_MBUTTONUP,						&CEngine::HandleMsg_ButtonUp },

	{ WM_CLOSE,							&CEngine::HandleMsg_Close },
	{ WM_QUIT,							&CEngine::HandleMsg_Close }
};

//////////////////////////////////////////////////////////////////////////
LRESULT WINAPI EngineWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return gEngine.WindowProc( hWnd, uMsg, wParam, lParam );
}

//////////////////////////////////////////////////////////////////////////
CEngine::CEngine()
{
	frametime = oldrealtime = realtime = 0.0f;
	quit = false;
	fps = 0;
	m_fnFrameFunc	= NULL;
	gD3D = pD3D			= Direct3D::Instance();
	minimized		= false;
}

CEngine::~CEngine()
{
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Init()
{
	trace("Initializing Engine");
	CShaderFactory::LoadShaders();
	gRenderer.Init( pD3D->GetDevice() );	
	ilInit();
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Shutdown()
{
	trace("Shutting down engine");
	gRenderer.Release();
	//gResourceMgr.Release();
	ShutdownDirectX();
	CModelLoader::Release();
	UnregisterClass( CLASSNAME, wndclass.hInstance );
}

//////////////////////////////////////////////////////////////////////////
bool CEngine::CreateEngineWindow( const char *pszTitle )
{
	return CreateEngineWindowEx( pszTitle, cv_WindowWidth.GetInt(), cv_WindowHeight.GetInt(), cv_BPP.GetInt(), cv_Fullscreen.GetBool(), EngineWindowProc );
}

bool CEngine::CreateEngineWindowEx( const char *pszTitle, int iWidth, int iHeight, int iBits, bool bFullscreen, FnWndProc fnWndProc )
{
	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, EngineWindowProc, 0, 0,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		CLASSNAME, NULL
	};

	wndclass = wc;
	RegisterClassEx( &wndclass );

	width = iWidth;
	height = iHeight;

	// Create the application's window
	hWnd = CreateWindow(	CLASSNAME, "VoidEngine",
							//WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION,				// z paskiem i tytulem
							WS_POPUP,
							0, 0, width, height,
							NULL, NULL, wndclass.hInstance, NULL );

	CenterWindow();
	InitDirectX();

	// Show the window
	ShowWindow( hWnd, SW_SHOWDEFAULT );
	UpdateWindow( hWnd );

	if (bFullscreen)
		gD3D->pSwapChain->SetFullscreenState(true, NULL);

	// Initialize all systems
	Init();

	return true;	
}

//////////////////////////////////////////////////////////////////////////
void CEngine::InitDirectX()
{
	if (!pD3D->Init(width, height, false, hWnd, false, 1000.0f, 1.0f ) )
		MessageBox(NULL,"Error while initializing Direct3D","ERROR",0);
}

//////////////////////////////////////////////////////////////////////////
void CEngine::ShutdownDirectX()
{
	if (pD3D) pD3D->Shutdown();
}

//////////////////////////////////////////////////////////////////////////
void CEngine::CenterWindow( void )
{
	// wycentruj okno
	int     CenterX, CenterY;

	CenterX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	CenterY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	CenterX = (CenterX < 0) ? 0: CenterX;
	CenterY = (CenterY < 0) ? 0: CenterY;

	SetWindowPos (hWnd, NULL, CenterX, CenterY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Quit()
{
	quit = true;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CEngine::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG			lRet = 0;
	HDC				hdc;
	PAINTSTRUCT		ps;
	RECT			windowRect;

	// Jesli wychodzimy z programu, oblsuz tylko podstawowe komunikaty
	if ( quit )
	{
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	AddGameMessage( uMsg, wParam, lParam );

	switch (uMsg)
	{
	case WM_CREATE:
		::SetForegroundWindow(hWnd);
		break;

	case WM_SYSCOMMAND:
		if ( wParam == SC_CLOSE ) 
		{
			quit = true;
			return lRet;
		}
		break;

	case WM_ACTIVATE:
	case WM_SETFOCUS:
		hasfocus = true;
		break;

	case WM_KILLFOCUS:
		hasfocus = false;
		
	case WM_MOVE:
		GetWindowRect( hWnd, &windowRect );
		gInput.SetWindowData( windowRect.left, windowRect.top, windowRect.right-windowRect.left, windowRect.bottom-windowRect.top );
		break;

	case WM_SIZE:
		if ( LOWORD(wParam) == SIZE_MINIMIZED )
		{		
			minimized = true;
			break;
		}

		if ( LOWORD(wParam) == SIZE_RESTORED )
		{		
			minimized = false;
			break;
		}

		
		break;

	case WM_PAINT:
		break;

	default:
		break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
FnWndProc CEngine::GetWindowProc( void )
{
	return EngineWindowProc;
}

//////////////////////////////////////////////////////////////////////////
void CEngine::AddGameMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CStoredGameMessage stored;
	stored.uMsg = uMsg;
	stored.wParam = wParam;
	stored.lParam = lParam;
	m_StoredGameMessages.AddToTail( stored );
}

//////////////////////////////////////////////////////////////////////////
void CEngine::DispatchStoredGameMessages( void )
{
	while ( m_StoredGameMessages.Size() > 0 )
	{
		DispatchGameMsg( m_StoredGameMessages[0].uMsg, m_StoredGameMessages[0].wParam, m_StoredGameMessages[0].lParam );
		m_StoredGameMessages.RemoveAt(0);
	}

}

//////////////////////////////////////////////////////////////////////////
void CEngine::DispatchGameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	for ( int i=0; i < ARRAYSIZE( g_GameMessageHandlers ); i++ )
	{
		if ( g_GameMessageHandlers[i].uMsg == uMsg )
		{
			(this->*g_GameMessageHandlers[i].pFn)( uMsg, wParam, lParam );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int	CEngine::GetWindowX(void) { return gInput.m_iWindowX; };
int	CEngine::GetWindowY(void) { return gInput.m_iWindowY; };

//////////////////////////////////////////////////////////////////////////
// CALLBACKS
//////////////////////////////////////////////////////////////////////////
void CEngine::HandleMsg_UIMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.KeyEvent( ( ( int ) wParam ) > 0 ? K_MWHEELUP : K_MWHEELDOWN, true );
	gInput.KeyEvent( ( ( int ) wParam ) > 0 ? K_MWHEELUP : K_MWHEELDOWN, false );
};

void CEngine::HandleMsg_MouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.KeyEvent( ( short ) HIWORD( wParam ) > 0 ? K_MWHEELUP : K_MWHEELDOWN, true );
	gInput.KeyEvent( ( short ) HIWORD( wParam ) > 0 ? K_MWHEELUP : K_MWHEELDOWN, false );
};

void CEngine::HandleMsg_WMMove( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
};

void CEngine::HandleMsg_ActivateApp( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
};

void CEngine::HandleMsg_KeyDown( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.KeyEvent( MapScanCodeToEngineKey( lParam ) , true );
};

void CEngine::HandleMsg_KeyUp( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.KeyEvent( MapScanCodeToEngineKey( lParam ) , false );
};

void CEngine::HandleMsg_MouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
};

void CEngine::HandleMsg_ButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.MouseEvent( FieldsFromMouseWParam( wParam ), true );
};

void CEngine::HandleMsg_ButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	gInput.MouseEvent( FieldsFromMouseWParam( wParam ), false );
};

void CEngine::HandleMsg_Close( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	quit = true;
};

void CEngine::HandleMsg_Quit( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
};

//////////////////////////////////////////////////////////////////////////
int CEngine::MapScanCodeToEngineKey(LPARAM lParam)
{
	int result;
	int scanCode = ( lParam >> 16 ) & 255;
	bool is_extended = (lParam & ( 1 << 24 )) != 0;

	if ( scanCode > 127)
		return 0;

	if ( lParam & ( 1 << 24 ) )
		is_extended = true;

	result = s_pScanToEngine[scanCode];

	if ( !is_extended )
	{
		switch ( result )
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			break;
		}
	}
	else
	{
		switch ( result )
		{
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		}
	}

	return result;
}


int CEngine::FieldsFromMouseWParam( WPARAM wParam )
{
	int temp = 0;

	if (wParam & MK_LBUTTON)
	{
		temp |= 1;
	}

	if (wParam & MK_RBUTTON)
	{
		temp |= 2;
	}

	if (wParam & MK_MBUTTON)
	{
		temp |= 4;
	}

	return temp;
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Loop()
{
	static double newtime, oldtime;
	double time;

	while ( 1 )
	{
		MSG msg;
		framegraph.Update(frametime);
		
		// obsluz standardowe wiadomosci windowswoe
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		
		// odpal ich odpowiedniki
		DispatchStoredGameMessages();
		
		// Konczymy aplikacje jesli quit
		if ( quit )
			break;

		// policz czas od ostatniego cyklu
		newtime = GetFloatTime();
		time = newtime - oldtime;
		oldtime = newtime;

		// odpal klatke silnika
		Frame( time );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEngine::FilterTime( double fTime )
{
	realtime += fTime;							// Zwieksz czas rzeczywisty
	

	// Dedicated's tic_rate regulates server frame rate.  Don't apply fps filter here.
	float fps = fps_max.GetFloat();
	if ( fps != 0 )
	{
		// Limit fps to within tolerable range
		fps = max( 0.1f, fps );
		fps = min( 1000.0f, fps );

		float minframetime = 1.0 / fps;

		if (( realtime - oldrealtime ) < minframetime )
		{
			// framerate is too high
			return false;		
		}
	}

	frametime = realtime - oldrealtime;
	oldrealtime = realtime;

	static float accumulator = 0;

	accumulator += frametime;
	if (accumulator < 0.0001) return false;

	frametime = accumulator;
	accumulator = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Display()
{
	if (pD3D) pD3D->Display();
}

//////////////////////////////////////////////////////////////////////////
void CEngine::Frame( double fTime )
{
	if ( !FilterTime( fTime ) )
		return;

	CountFPS();

	gInput.DefaultWindowUpdate( hWnd );
	gConsole.Execute();
	orthoMatrix = Matrix4::MakeOrtho(0, width, 0, height, 0, 1);	

	if ( m_fnFrameFunc )
		m_fnFrameFunc( frametime );
	
	gInput.Frame();
}

//////////////////////////////////////////////////////////////////////////
void CEngine::CountFPS()
{
	// Liczenie FPS
	static int frames = 0;
	static float timer = 0.0f;

	float t2 = GetFloatTime();

	
	if ( t2 > timer )
	{
		timer = t2+1.0f;
		fps = frames;
		frames = 0;
	}
	frames++;
}

//////////////////////////////////////////////////////////////////////////
void CEngine::SetFrameFunc(FnFrameFunc fnFunc)
{
	m_fnFrameFunc = fnFunc;
}


