#pragma once
#include <Windows.h>
#include <string>
#include "platform.h"
#include "Array.h"
#include "Utils.h"

typedef int			VoidFont;

#define	FONT_DEFAULT				0
#define MAX_FONTS					32

//////////////////////////////////////////////////////////////////////////
// Klasa menedzera czcionek, implementacja interfejsu IFont
//////////////////////////////////////////////////////////////////////////
class CFontMgr
{
public:

	CFontMgr();
	~CFontMgr();

	void		Init();																						// Tworzy defaultowa czcionke
	void		Release();

	VoidFont	LoadFont( const char *szName, unsigned int iSize, bool bBold = 0, bool bItalic = 0,  bool bAntialiased = 0 );		// Laduje podana czcionke i zwraca jej indeks
	VoidFont	LoadFontFromFile( const char *szFile, const char *szFacename, unsigned int iSize, bool bBold = 0, bool bItalic = 0,  bool bAntialiased = 0 );		
	
	void		GetTextSize( VoidFont font, const char* pstr, float &xOut, float &yOut );
	int			GetSize( VoidFont font );
	void		DrawString( float x, float y, VoidFont font, const char *pstr, ... );
	void		DrawString( float x, float y, VoidFont font, SRGBA color, const char *pstr, ... );
	void		Print( float x, float y, SRGBA color, const char *text, ... );

	bool		IsValid( VoidFont font );

public:

	bool				m_bInitialized;

	LPD3DX10FONT		fonts[MAX_FONTS];				// Lista obiektow czcionek
	byte				fontsnum;
	CArray<string>		fontreslist;					// Lista FontResource, ktore nalezy zwalniac po wyjsciu	
};

extern CFontMgr	gFontMgr;