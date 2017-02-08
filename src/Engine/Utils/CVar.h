#pragma once

#include "platform.h"
#include "Array.h"


class CCommandBase;
class CCommand;
class CVar;
class CCommandMgr;

// flagi dotyczace CVar i CCommand
#define FCVAR_UNREGISTERED	(1<<0)		// nie dodawaj do listy, ktora trafi potem do CVars
#define FCVAR_ENGINE		(1<<1)		// zmienna konfiguracyjna, niemozliwa do zmiany przez uzytkownika
#define FCVAR_EXTDLL		(1<<2)		// przez pozostalego dlla
#define FCVAR_CLIENTDLL		(1<<3)		// przez client.dll
#define FCVAR_CHEAT			(1<<4)		// cheat, tylko w SP lub w MP z sv_cheats "1"

// tylko CVar
#define FCVAR_PROTECTED		(1<<5)		// zmienna servera, nie wysylana przy zadaniu z racji utajnienia (np. password)
#define FCVAR_SPONLY		(1<<6)		// tylko dla SP (nie mozna jej zmienic przez clienta polaczonego z serverem)
#define FCVAR_ARCHIVE		(1<<7)		
#define FCVAR_SERVER		(1<<8)		// zmiana oznajmiana wszystkim clientom
#define FCVAR_USERINFO		(1<<9)		// zmiana danych podlaczonego clienta
#define FCVAR_PRINTABLEONLY	(1<<10)		// musi zawierac wylacznie drukowalne znaki
#define FCVAR_REPLICATED	(1<<11)



// funckja wywolywana przy zmianie cvar
typedef void ( *FnChangeFunc )( CVar *var, char const *pOldString );

// funkcja wywolywana przy odpaleniu ccommand
typedef void ( *FnCommandFunc )( void );


//////////////////////////////////////////////////////////////////////////
// Klasa bazowa
//////////////////////////////////////////////////////////////////////////
class CCommandBase
{
public:
	CCommandBase();
	~CCommandBase() {};

	static int				CountCVars( void );					// Policz zadeklarowane w tym dll'u cvar'y

	char const				*GetName( void );					// Zwraca nazwe zmiennej/funckji
	char const				*GetHelp( void );					// Zwraca tekst pomocy

	static CCommandBase		*GetCommands( void );				// Zwraca pierwsza komende na liscie
	CCommandBase			*GetNext( void );					// Zwraca nastepny element 
	static CCommandBase		*FindCommand( char const *name );
	static CCommandBase		*FindCommandPart( char const *name, std::vector<CCommandBase*> &ret );

	virtual bool			IsCommand( void );

	bool					IsFlagSet( int flag );
	void					AddFlag( int flag );


protected:
	void					Create( char const *pName, char const *pHelp = 0, int flags = 0 );

public:
	CCommandBase			*m_pNext;		// nastepny element listy tymczasowej, uzywany przez managera
	static CCommandBase		*s_pBase;

	int						m_iFlags;		// flagi

private:
	bool					m_bRegistered;	// czy juz zarejestrowalismy zmienna w cvars

	char const				*m_pszName;		// nazwa identyfikujaca zmienna
	char const				*m_pszHelp;		// tekst help

};

//////////////////////////////////////////////////////////////////////////
// CCommand
//////////////////////////////////////////////////////////////////////////
class CCommand : public CCommandBase
{
public:
	typedef CCommandBase BaseClass;

	CCommand( void );
	CCommand( char const *pName, FnCommandFunc func, char const *pHelp = 0, int flags = 0 );

	virtual					~CCommand( void );
	virtual bool			IsCommand( void );
	virtual void			Dispatch( void );

private:
	virtual void			Create( char const *pName, FnCommandFunc func, char const *pHelp = 0, int flags = 0 );

private:
	FnCommandFunc			m_fnCommandFunc;					// funkcja dla danej komendy
};

//////////////////////////////////////////////////////////////////////////
// CVar
//////////////////////////////////////////////////////////////////////////
class CVar : public CCommandBase
{
public:
	typedef CCommandBase BaseClass;

	CVar(char const *pName, char const *pDefaultValue, int flags = 0);

	CVar( char const *pName, char const *pDefaultValue, int flags, 
		char const *pHelp );
	CVar( char const *pName, char const *pDefaultValue, int flags, 
		char const *pHelp, bool bMin, float fMin, bool bMax, float fMax );
	CVar( char const *pName, char const *pDefaultValue, int flags, 
		char const *pHelp, FnChangeFunc func );
	CVar( char const *pName, char const *pDefaultValue, int flags, 
		char const *pHelp, bool bMin, float fMin, bool bMax, float fMax, FnChangeFunc func );


	virtual					~CVar ( void );

	virtual bool			IsCommand( void );

	float					GetFloat( void );				// pobieranie wartosci
	int						GetInt( void );
	bool					GetBool( void ) {  return !!GetInt(); }
	char const				*GetString( void );

	bool					GetMin( float& value );
	bool					GetMax( float& value );

	bool					SetValue( char const *value );
	bool					SetValue( float value );
	bool					SetValue( int value );

	void					Reset( void );					// przywroc wartosc domyslna
	void					ResetAll( void );

	bool					ClampValue( float& value );

private:

	virtual void			Create( char const *pName, char const *pDefaultValue, int flags = 0,
									char const *pHelp = 0, bool bMin = false, float fMin = 0.0,
									bool bMax = false, float fMax = false, FnChangeFunc func = 0 );

private:
	char const			*m_pszDefaultValue;	

	char				m_pszString[32];	
	int					m_StringLength;		

	float				m_fValue;			
	int					m_iValue;

	bool				m_bHasMin;			
	float				m_fMinVal;
	bool				m_bHasMax;
	float				m_fMaxVal;

	FnChangeFunc		m_fnChangeFunc;			// funkcja dla zmiany wartosci
};



//////////////////////////////////////////////////////////////////////////
// Lokalny menedzer zmiennych cvar
// Spamietuje zadeklarowane w danym module zmienne, po czym przy ladowaniu
// interfejsu silnika, dolaczamy go do IConsole za pomoca IConsole::RegisterCommandList
//////////////////////////////////////////////////////////////////////////
class CCommandMgr
{
private:

	CCommandMgr( void ) {};

public:

	~CCommandMgr( void )
	{ 
	}

	static void AddCommand( CCommandBase *pCmd )
	{
		if (!s_pInstance)	s_pInstance = new CCommandMgr();

		s_pInstance->m_CommandList.AddToTail(pCmd);
	}

	static CArray<CCommandBase*> *GetList()
	{
		if (!s_pInstance)	s_pInstance = new CCommandMgr();

		return &s_pInstance->m_CommandList;
	}

public:
	static CCommandMgr				*s_pInstance;
	CArray<CCommandBase*>			m_CommandList;
};
