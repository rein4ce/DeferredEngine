#include "platform.h"
#include "Engine.h"
#include "ConsolePanel.h"
#include "CVar.h"
#include "VoidGame.h"
#include <gl/gl.h>
#include "Framework.h"
#include "VertexFormats.h"
#include <d3dx10.h>
#include "Renderer.h"
#include "Resources.h"
#include "Font.h"
#include "Sprite.h"
#include "VGUI.h"
#include "Input.h"
#include "Console.h"

CConsolePanel gConsolePanel;

CVar cv_ConsoleSize( "consolesize", "0" );			// Wysokosc panelu konsoli (0 - caly ekran)
CVar cv_ShowConsole( "consoleshow", "0" );
CVar cv_ConsoleDebug( "consoledebug", "1" );		// Powoduje, ze ostatnie linie konsoli pojawiaja sie na gorze ekranu gdy ta jest wylaczona

#define SERVERCOLOR				SRGB(120,120,65)
#define CLIENTCOLOR				SRGB(80,80,35)
#define ERRORCOLOR				SRGB(255,70,70)
#define DEBUGCOLOR				SRGB(196,181,80)

void CmdToggleConsole_f()
{
	if ( cv_ShowConsole.GetBool() )
		gConsolePanel.Hide();
	else
		gConsolePanel.Show();
}

CCommand cmd_ToggleConsole( "toggleconsole", CmdToggleConsole_f );

SStoredConsoleText::SStoredConsoleText(char *t, SRGB c) 
{ 
	text.assign(t), color = c; 
	time = gEngine.GetTime(); 
};

//////////////////////////////////////////////////////////////////////////
CConsolePanel::CConsolePanel()
{
	opacity = 0;
}

CConsolePanel::~CConsolePanel()
{

}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::Init( void )
{
	pBackground = gRenderer.AddSpriteRect(CTexture::Get("textures/console.jpg"), 0, 0, gEngine.width, gEngine.height, SRGBA(55,55,55,0));
	pBackground->z = 99999;
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::RenderDebugMode( void )
{
	int i,j,h;
	h = gFontMgr.GetSize( 0 );

	if ( m_ConsoleHistory.Size()==0 )
		return;

	i = m_ConsoleHistory.Size()-CONSOLE_DEBUG_LINES;

	if (i<0)
		i = 0;

	for (j=0; j<CONSOLE_DEBUG_LINES && i<m_ConsoleHistory.Size(); j++, i++)
	{
tryagain:
		// Jesli czas wyswietlania tej linii juz uplynal, skip
		if ( gEngine.GetTime() > m_ConsoleHistory[i].time + CONSOLE_DEBUG_LINE_FADE_TIME )
		{
			i++;
			if (i == m_ConsoleHistory.Size())
				return;

			goto tryagain;
		}

		gVGUI.AddTextMessageEx( 10, 5+h*j, SRGB(255,255,255), 0, true, 255, m_ConsoleHistory[i].text.c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::Render( void )
{
	int lines = 0, h, i, j, hpos;

	// Jesli niewidoczna a tryb console-debug ostatnie linie konsoli pojawiaja sie na gorze ekranu
	if ( cv_ShowConsole.GetBool()==false && cv_ConsoleDebug.GetBool() )
	{
		RenderDebugMode();
	}

	// Jesli widoczna, zwieksz opacity
	if ( cv_ShowConsole.GetBool() )
	{
		opacity += (gFramework.frametime / CONSOLE_FADETIME);
		if ( opacity > 1.0f )
			opacity = 1.0f;
	} else
	{
		opacity -= (gFramework.frametime / CONSOLE_FADETIME);
		if ( opacity < 0.0f )
			opacity = 0.0f;
	}

	pBackground->color = SRGBA(55,55,55, 255.0f * opacity);

	if ( !cv_ShowConsole.GetBool() && opacity == 0.0f )
		return;

	// Policz ilosc wyswietlanych linii
	if ( cv_ConsoleSize.GetInt() == 0 )
	{
		h = gEngine.GetHeight();
	} else
	{
		h = cv_ConsoleSize.GetInt();
	}
	lines = ceil( (double)h / (double)gFontMgr.GetSize( 0 ) ) - 1;		// Pomniejszamy o 1 zeby miec miejsce dla wpisywanego tekstu


	static int width = gEngine.GetWidth();
	static int height = gEngine.GetHeight();

	// Tlo konsoli
	SVertexColorTextured2D vert[4] = {
		{ 0,		0,		1, 1, 0x33000000, 0, 0 },
		{ width,	0,		1, 1, 0xFFFFFFFF, 1, 0 },
		{ width,	height, 1, 1, 0xFFFFFFFF, 1, 1 },
		{ 0,		height, 1, 1, 0xFFFFFFFF, 0, 1 }
	};
	
	h-=12;

	if ( m_ConsoleHistory.Size() == 0 )
		return;

	// Rysujemy text historii konsoli
	for ( j=0, i=m_ConsoleHistory.Size()-1, hpos = h-25; i>-1 && j<lines+1; i--, j++, hpos-=gFontMgr.GetSize( 0 ))
	{
		gFontMgr.DrawString( 10, hpos, 0, SRGBA(m_ConsoleHistory[i].color.r,m_ConsoleHistory[i].color.g,m_ConsoleHistory[i].color.b,(byte)(opacity*255.0f) ), m_ConsoleHistory[i].text.c_str() );
	}

	// Rysujemy wpisywany text
	m_strInputLine.assign( gInput.GetTypingText() );			// Najpierw musimy zupdateowac text 
	gFontMgr.DrawString( 10, h-5, 0, SRGBA(255,70,70,(byte)(opacity*255.0f)), m_strInputLine.c_str() );

	// Prompt
	if ( ((int)(gEngine.GetTime()*1000.0f))%500 < 250 )
	{
		float w,_h;
		gFontMgr.GetTextSize(0,m_strInputLine.c_str(),w,_h);
		gFontMgr.DrawString( 10+w, h-5, 0, SRGBA(255,70,70,(byte)(opacity*255.0f)), "_" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::AddLine( char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddLine( SRGB(255,255,255), pBuffer );
	va_end(args);
}

void CConsolePanel::AddErrorLine( char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddLine( ERRORCOLOR, pBuffer );
	va_end(args);
}

void CConsolePanel::AddServerLine( char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddLine( SERVERCOLOR, pBuffer );
	va_end(args);
}

void CConsolePanel::AddClientLine( char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddLine( CLIENTCOLOR, pBuffer );
	va_end(args);
}

void CConsolePanel::AddDebugLine( char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	AddLine( DEBUGCOLOR, pBuffer );
	va_end(args);
}


void CConsolePanel::AddLine( SRGB color, char *text, ... )
{
	char pBuffer[1024];
	va_list args;
	va_start( args, text );
	vsprintf( &pBuffer[0], text, args );
	m_ConsoleHistory.AddToTail( SStoredConsoleText( pBuffer, color ) );
	va_end(args);

	OutputDebugString(pBuffer);

	if ( m_ConsoleHistory.Size() >= MAX_CONSOLE_HISTORY )
		m_ConsoleHistory.RemoveAt(0);
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::Show()
{
	cv_ShowConsole.SetValue(1);
	m_strInputLine.clear();
	gInput.BeginTyping( "", EnterFunc );
}

void CConsolePanel::Hide()
{
	cv_ShowConsole.SetValue(0);
	gInput.EndTyping();
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::EnterFunc()
{
	char str[1024];
	sprintf( str, gInput.GetTypingText() );
	if ( strlen(str)==0 )										// Jesli pusta linia, nic nie robi
		return;
	gConsole.AddCommand( str );							// Wykonuje zadane polecenie
	gConsolePanel.AddLine( str );								// Dodaje do historii wykonane polecenie
	gConsolePanel.m_strInputLine.clear();						// Czysci linijke
	gInput.BeginTyping( "", EnterFunc );					// Rozpoczyna pisanie nowego tekstu
}

//////////////////////////////////////////////////////////////////////////
void CConsolePanel::Clear()
{
	m_ConsoleHistory.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
bool CConsolePanel::IsVisible()
{
	return cv_ShowConsole.GetInt() == 1;
}
