#pragma once
#include "Font.h"
#include "Utils.h"
#include "Array.h"
#include "Config.h"

namespace GUI
{
	class Panel;
	struct Style;
};


// Zachowany w pamieci tekst do wyswietlenia na ekranie
struct SStoredText
{
	string		text;
	int			x;
	int			y;
	SRGB		color;
	VoidFont	font;
	bool		shadow;
	byte		opacity;

	SStoredText() 
	{ 
		x=0; 
		y=0; 
		color = SRGB(255,255,255); 
		font=0; 
		shadow=true; 
		opacity=255; 
	};
	~SStoredText() {};
};

#define PrintText gVGUI.AddTextMessage

//////////////////////////////////////////////////////////////////////////
// Klasa do zarzadzania interfejsem graficznym
// posiada funkcje m.in. do wyswietlania tekstu na ekranie
//////////////////////////////////////////////////////////////////////////
class CVGUI
{
public:
					CVGUI( void );
	virtual			~CVGUI( void );

	void			Init( void );
	void			Release( void );
	void			Frame(float frametime, float realtime);
	void			Render( void );
	void			LoadStyles( void );
	
	void			AddTextMessage(const char *text, ... );
	void			AddTextMessage(SRGB color, const char *text, ... );
	void			AddTextMessage( int x, int y, const char *text, ... );
	void			AddTextMessage( int x, int y, SRGB color, const char *text, ... );
	void			AddTextMessageEx( int x, int y, SRGB color, VoidFont font, bool shadow, byte opacity, const char *text, ... );
	void			DrawTextMessages( void );

	void			CreateEntityPropertiesPanel();

	inline float			GetNextZindex() { guiZindex -= 0.0001f; return guiZindex; };
	inline void				ResetZindex() { guiZindex = 0.9999f; };

public:

	CArray<SStoredText>		m_StoredTextMessages;				
	CArray<GUI::Panel*>		elements;			
	float					guiZindex;
	int						cursor;
	ConfigFile				styles;

private:
	int				lastX, lastY;
};

extern CVGUI gVGUI;