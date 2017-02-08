#include "platform.h"
#include "Font.h"
#include "Engine.h"
#include "Array.h"
#include "Direct3D.h"

VoidFont g_DefaultFont;
CFontMgr			gFontMgr;

//////////////////////////////////////////////////////////////////////////
// CFontMgr
//////////////////////////////////////////////////////////////////////////
CFontMgr::CFontMgr()
{
	m_bInitialized = false;
	fontsnum = 0;
	for (int i=0;i<MAX_FONTS;i++)
		fonts[i] = NULL;
}

CFontMgr::~CFontMgr()
{
}

void CFontMgr::Release()
{
	for (int i=0; i<fontreslist.Size(); i++)
	{
		RemoveFontResource( fontreslist[i].c_str() );		
	}

	for (int i=0;i<MAX_FONTS;i++)
		if (fonts[i]!=NULL)
		{
			fonts[i]->Release();
			fonts[i] = NULL;
		}
}

//////////////////////////////////////////////////////////////////////////
VoidFont CFontMgr::LoadFontFromFile( const char *szFile, const char *szFacename, unsigned int iSize, bool bBold, bool bItalic, bool bAntialiased )
{
	if (!AddFontResource(szFile))
	{
		Error("Unable to load font %s", szFile);
		return 0;
	}
	fontreslist.AddToHead( string(szFile) );

	int id = fontsnum;
	fontsnum++;

	D3DX10CreateFont( gEngine.GetD3D()->GetDevice(),// D3DDevice
		iSize, 0,							// Width, Height
		(bBold)?(FW_BOLD):(FW_LIGHT),		// Weight
		0,									// MipLevels
		bItalic,							// Italic
		DEFAULT_CHARSET,					// Charset
		OUT_DEFAULT_PRECIS,					// Precision
		(bAntialiased)?(CLEARTYPE_NATURAL_QUALITY):(NONANTIALIASED_QUALITY),	// Quality
		DEFAULT_PITCH || FF_DONTCARE,		// Pitch
		TEXT(szFacename),					// Facename
		&fonts[id]							// Pointer to LPD3DXFONT
	);
	return id;
}


//////////////////////////////////////////////////////////////////////////
VoidFont CFontMgr::LoadFont( const char *szName, unsigned int iSize, bool bBold, bool bItalic, bool bAntialiased )
{
	if ( fontsnum == MAX_FONTS-1 )
	{
		Error("Max fonts reached");
		return 0;
	}

	int id = fontsnum;
	fontsnum++;
	
	
					D3DX10CreateFont( gEngine.GetD3D()->GetDevice(),		// D3DDevice
										iSize, 0,							// Width, Height
										(bBold)?(FW_BOLD):(FW_LIGHT),		// Weight
										0,									// MipLevels
										bItalic,							// Italic
										DEFAULT_CHARSET,					// Charset
										OUT_DEFAULT_PRECIS,					// Precision
										(bAntialiased)?(DEFAULT_QUALITY):(NONANTIALIASED_QUALITY),	// Quality
										DEFAULT_PITCH || FF_DONTCARE,		// Pitch
										TEXT(szName),						// Facename
										&fonts[id]							// Pointer to LPD3DXFONT
										);
					
	return id;
}

//////////////////////////////////////////////////////////////////////////
bool CFontMgr::IsValid( VoidFont font )
{
	if ( font < 0 || font >= fontsnum )
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////

void CFontMgr::GetTextSize( VoidFont font, const char* pstr, float &xOut, float &yOut )
{
	if ( !IsValid(font) )
		return;

	RECT rct;
	fonts[font]->DrawText(NULL, pstr, -1, &rct, DT_LEFT | DT_CALCRECT, 0xFFFFFFFF );
	xOut = rct.right - rct.left;
	yOut = rct.bottom - rct.top;
}

//////////////////////////////////////////////////////////////////////////
int CFontMgr::GetSize( VoidFont font )
{
	if ( !IsValid(font) )
		return 0;

	return 12;
}

//////////////////////////////////////////////////////////////////////////
void CFontMgr::DrawString( float x, float y, VoidFont font, const char *pstr, ... )
{
	if ( !m_bInitialized )
		return;

	if ( !IsValid(font) )
		font = 0;
	
	char pBuffer[1024];
	va_list args;
	va_start( args, pstr );
	vsprintf( &pBuffer[0], pstr, args );
	va_end(args);

	D3DXCOLOR color = D3DXCOLOR( 0xFFFFFF );    

	// Create a rectangle to indicate where on the screen it should be drawn
	RECT rct;
	rct.left = x;
	rct.top = y;
	rct.right = gEngine.GetWidth();
	rct.bottom = gEngine.GetHeight();

	// Draw some text 
	fonts[font]->DrawText(NULL, pBuffer, -1, &rct, DT_LEFT, color );
}

//////////////////////////////////////////////////////////////////////////
void CFontMgr::Print( float x, float y, SRGBA color, const char *text, ... )
{
	DrawString( x, y, g_DefaultFont, color, text );
}

//////////////////////////////////////////////////////////////////////////
void CFontMgr::DrawString( float x, float y, VoidFont font, SRGBA color, const char *pstr, ... )
{
	if ( !m_bInitialized )
		return;

	if ( !IsValid(font) )
		font = 0;

	char pBuffer[1024];
	va_list args;
	va_start( args, pstr );
	vsprintf( &pBuffer[0], pstr, args );
	va_end(args);

	D3DXCOLOR c = color;    

	// Create a rectangle to indicate where on the screen it should be drawn
	RECT rct;
	rct.left = x;
	rct.top = y;
	rct.right = gEngine.GetWidth();
	rct.bottom = gEngine.GetHeight();

	fonts[font]->DrawText(NULL, pBuffer, -1, &rct, DT_LEFT, c );
}

//////////////////////////////////////////////////////////////////////////
void CFontMgr::Init()
{
	Assert( !m_bInitialized );

	g_DefaultFont = LoadFont( "Verdana", 12 );
	m_bInitialized = true;
}

