#pragma once
#include "platform.h"
#include "Array.h"
#include "Utils.h"

class CSprite2D;

#define MAX_CONSOLE_HISTORY				128					// Ile linijek tekstu pamieta konsola
#define CONSOLE_BACKCOLOR				220
#define CONSOLE_FADETIME				0.3f				// W sekundach
#define CONSOLE_DEBUG_LINES				5					// Ile ostatnich linii tekstu wyswietlac jesli tryb debug
#define CONSOLE_DEBUG_LINE_FADE_TIME	5.0f				// Ile sekund wyswietla sie linia w trybie debug

// Zapamietana linijka tekstu w konsoli
struct SStoredConsoleText
{
	string		text;
	SRGB		color;
	double		time;

	SStoredConsoleText() {};
	SStoredConsoleText(char *t, SRGB c);
};

//////////////////////////////////////////////////////////////////////////
// Panel konsoli wystepujacy tylko w grze, odbierajacy output debug
//////////////////////////////////////////////////////////////////////////
class CConsolePanel
{
public:
					CConsolePanel();
	virtual			~CConsolePanel();

	void			Init( void );
	void			Render( void );
	void			RenderDebugMode( void );

	void			AddLine( char *text, ... );
	void			AddErrorLine( char *text, ... );
	void			AddDebugLine( char *text, ... );
	void			AddServerLine( char *text, ... );
	void			AddClientLine( char *text, ... );
	void			AddLine( SRGB color, char *text, ... );
	void			Show( void );
	void			Hide( void );
	void			Clear( void );
	bool			IsVisible();

	static void		EnterFunc( void );

public:
	CArray<SStoredConsoleText>		m_ConsoleHistory;

	double							opacity;
	string							m_strInputLine;
	CSprite2D						*pBackground;
};

extern CConsolePanel gConsolePanel;

#define CONSOLE					gConsolePanel.AddLine
#define CONERROR				gConsolePanel.AddErrorLine
#define CONDBG					gConsolePanel.AddDebugLine