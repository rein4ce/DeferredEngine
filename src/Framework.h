#pragma once
#include "platform.h"

void GameFrameFunc( double fTime );

//////////////////////////////////////////////////////////////////////////
// Stan gry
//////////////////////////////////////////////////////////////////////////
enum EGameState
{
	GAMESTATE_MAINMENU,							// Meny glowne
	GAMESTATE_GAME								// Gra
};

//////////////////////////////////////////////////////////////////////////
class CFramework
{
public:

	CFramework();
	~CFramework();

	// Glowne funkcje
	bool Init( void );
	void Frame( double fTime );										// Funkcja klatki gry
	void Shutdown( void );

	void UpdateGUI();
	
	// Zarzadzanie stanem gry
	void CreateGame( const char *pszMap );							// Tworzy nowa gre na podstawie domyslnych parametrow 
	void EndGame( void );											// Konczy gre i wraca do menu

public:
	EGameState				state;

	double					frametime;								// Kopia czasu klatki silnika
	double					realtime;
};

extern CFramework	gFramework;

