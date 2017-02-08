#pragma once

#include "platform.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#include "Array.h"
#include "Matrix.h"

#ifdef EDITOR_BUILD
#define CLASSNAME			"VoidEditor"
#endif

#ifdef GAME_BUILD
#define CLASSNAME			"VoidGame"
#endif

#ifndef CLASSNAME
#define CLASSNAME			"Void"
#endif

class CEngine;
class CGameMessageHandler;
class CStoredGameMessage;
class CFrameGraph;
class Direct3D;

//////////////////////////////////////////////////////////////////////////
// Handle dla eventu okna Win32
//////////////////////////////////////////////////////////////////////////
class CGameMessageHandler
{
public:
	UINT			uMsg;
	void			(CEngine::*pFn)( UINT uMsg, WPARAM wParam, LPARAM lParam );
};

//////////////////////////////////////////////////////////////////////////
// Klasa zachowanej wiadomosci okna
//////////////////////////////////////////////////////////////////////////
class CStoredGameMessage
{
public:
	UINT	uMsg;
	WPARAM	wParam;
	LPARAM	lParam;
};

typedef void ( *FnFrameFunc )( double );

//////////////////////////////////////////////////////////////////////////
// Glowna klasa silnika gry
// Klasa ta obsluguje okno gry, zarzadza tworzeniem i usuwaniem watku
// oraz posiada funkcje petli
//////////////////////////////////////////////////////////////////////////
class CEngine
{
public:
	
	CEngine();
	virtual ~CEngine();

	// Kontrola aplikacji
	void		Init();															// Initialize engine subsystems
	void		Frame( double fTime );											// Klatka silnika
	void		Quit( void );
	void		Shutdown( void );												// Konczy dzialanie aplikacji	
	void		Loop( void );													// Glowna petla aplikacji
	void		SetFrameFunc( FnFrameFunc fnFunc );								// Przypisuje funkcje klatki

	// Glowna procedura okna
	LRESULT		WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	FnWndProc	GetWindowProc( void );											// Zwraca wskaznik na standardowa procedure okna

	// Funckje tworzenia okna
	bool		CreateEngineWindow( const char *pszTitle );						// Tworzy okno na podstawie zmiennych cvar (ladowanych z konfiguracja .cfg)
	bool		CreateEngineWindowEx( const char *pszTitle, int iWidth = 800, int iHeight = 600, int iBits = 32, bool bFullscreen = false, FnWndProc fnWndProc = NULL );
	void		CenterWindow( void );
	void		InitDirectX( void );
	void		ShutdownDirectX( void );
	void		Display( void );
	void		Clear( byte r, byte g, byte b );

	// Messahe handling
	void		AddGameMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		DispatchGameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		DispatchStoredGameMessages( void );

	// Win32 handlers
	void		HandleMsg_UIMouseWheel	( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_MouseWheel	( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_WMMove		( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_ActivateApp	( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_KeyDown		( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_KeyUp			( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_MouseMove		( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_ButtonDown	( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_ButtonUp		( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_Close			( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void		HandleMsg_Quit			( UINT uMsg, WPARAM wParam, LPARAM lParam );

	int			MapScanCodeToEngineKey(LPARAM lParam);
	int			FieldsFromMouseWParam( WPARAM wParam );

	// Utility functions
	bool		FilterTime( double fTime );
	void		CountFPS( void );
	
	// Accessors
	int				GetWindowX(void);
	int				GetWindowY(void);
	inline int		GetFPS( void )			{ return fps; };
	inline double	GetTime( void )			{ return realtime; };
	inline int		GetWidth( void )		{ return width; };
	inline int		GetHeight( void )		{ return height; };	
	inline bool		IsMinimized( void )		{ return minimized; };
	inline Direct3D*	GetD3D( void )		{ return pD3D; };
	inline HWND		GetHWND( void )			{ return hWnd; };
	inline bool		HasFocus( void )		{ return GetFocus() == hWnd; }
	
public:

	bool							hasfocus;
	bool							minimized;

	double							frametime;						// Czas trwania klatki
	double							realtime;						// Czas od rozpoczecia gry
	double							oldrealtime;
	int								fps;							// Ilosc klatek na sekunde

	bool							quit;							// Czy zamykamy aplikacje

	CArray<CStoredGameMessage>		m_StoredGameMessages;			// Zachowane wiadomosci okna
	FnFrameFunc						m_fnFrameFunc;					// Funkcja klatki

	WNDCLASSEX						wndclass;
	HWND							hWnd;							// Handle okna DirectX
	int								width;
	int								height;

	Direct3D						*pD3D;
	Matrix4							orthoMatrix;
};

extern CEngine		gEngine;
extern CFrameGraph	gFrameGraph;
extern Direct3D		*gD3D;

//////////////////////////////////////////////////////////////////////////
struct SFrame
{
	double frametime;
	double times[5];
	bool  on[5];
};

#define FRAMEGRAPH_HISTORY	150
struct SFramegraph
{
	CArray<SFrame>		history;

	int					x,y;
	byte				timesnum;
	double				start;

	//////////////////////////////////////////////////////////////////////////
	SFramegraph()
	{
		timesnum = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void Update(double d)
	{
		SFrame f;
		f.on[0] = f.on[1] = f.on[2] = 0;
		f.frametime = d;
		history.AddToTail(f);
		if (history.Size()>FRAMEGRAPH_HISTORY)
			history.RemoveAt(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void Render()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void Start()
	{
		start = GetFloatTime();
	}

	//////////////////////////////////////////////////////////////////////////
	void End(byte slot)
	{
		int i = history.Size()-1;
		history[i].on[slot] = 1;
		history[i].times[slot] = GetFloatTime()-start;
	}
};

extern SFramegraph		framegraph;
extern CEngine			gEngine;
