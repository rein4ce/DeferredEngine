#pragma once
#include "Common.h"
#include "Array.h"

#define CMD_BUFFER_SIZE			8192
#define MAX_ARGS				64

class CVar;
class CCommand;
class CCommandBase;

enum ECmdSource
{
	SRC_CLIENT,
	SRC_COMMAND
};

//////////////////////////////////////////////////////////////////////////
class CConsole
{
public:
					CConsole( void );
	virtual			~CConsole( void );

	void			SetOutputFunc( FnOutputPrint func );				// Ustawia funkcje wyjscia dla komunikatow (konsola,log,messagebox)
	void			Output( char *sText, ... );							// Wysyla text do wyswietlenia

	void			AddCommand( char *sCommand );						// Dodaje komende do wykonania
	void			AddCommandToHead( char *sCommand );					// Dodaje komende na poczatek listy
	void			Execute( void );									// Wykonuje zapamietane polecenia
	void			ExecuteString( char *sCommand );					// Wykonuje dane polecenie 

	void			RegisterCVar( CVar *pVar );							// Rejestruje zmienna cvar
	void			RegisterCommand( CCommand *pCommand );				// Rejestruje komende
	void			Init( CArray<CCommandBase*> *pList );				// Dodaje liste zmiennych (np. w modulach zewnetrznych)
	CVar*			GetCVar( const char *pName );						// Zwraca wskaznik na zmienna jesli istnieje lub NULL
	CCommand*		GetCommand( const char *pName );					// Zwraca wskaznik na funkcje jesli istnieje lub NULL
	void			WriteVariables( void );								// Zapisuje zmienne do archiwizacji do autoexec.cfg
	
	void			TokenizeString( char *text );						// Rozbija text na czlony
	char			*GetArgument( int iArg );							// Zwraca argument
	int				GetArgumentCount( void );							// Zwraca liczbe argumentow

public:

	int				m_iCmdArgc;
	char			m_szCmdBuffer[ CMD_BUFFER_SIZE ];
	char			*m_szCmdArgv[ MAX_ARGS ];
	char			*m_szCmdNullString;
	char			*m_szCmdArgs;
	char			m_szComToken[1024];

	sizebuf_t		m_CmdText;
	ECmdSource		m_CmdSource;

	CArray<CCommandBase*>	m_CommandList;								// Pelna lista komend i zmiennych cvar


	FnOutputPrint	m_fnOutputFunc;										// Funkcja wyswietlania komunikatow
};

extern CConsole gConsole;