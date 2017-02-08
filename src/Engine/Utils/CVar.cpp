#include "platform.h"
#include "CVar.h"



CCommandBase	*CCommandBase::s_pBase	= NULL;
CCommandMgr		*CCommandMgr::s_pInstance = NULL;



CCommandBase::CCommandBase()
{
	m_bRegistered	= 0;
	m_iFlags	= 0;
	m_pszName	= NULL;
	m_pszHelp	= NULL;
}

char const *CCommandBase::GetName()
{
	return m_pszName;
}

char const *CCommandBase::GetHelp()
{
	return m_pszHelp;
}

CCommandBase *CCommandBase::GetNext()
{
	return m_pNext;
}

bool CCommandBase::IsCommand()
{
	return true;
}

bool CCommandBase::IsFlagSet( int flag )
{
	return ( m_iFlags & flag ) ? 1 : 0;
}

void CCommandBase::AddFlag( int flag )
{
	m_iFlags |= flag;
}

CCommandBase *CCommandBase::GetCommands( void )
{
	return s_pBase;
}

//-----------------------------------------------------------------------------
// znajdz komende o podanej nazwie
//-----------------------------------------------------------------------------
CCommandBase *CCommandBase::FindCommand( char const *name )
{
	CCommandBase *cmd = GetCommands();
	Assert(cmd);
	for ( ; cmd; cmd = cmd->GetNext() )
	{
		if ( !stricmp( name, cmd->GetName() ) )
			return cmd;
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// znajdz komende o podanej nazwie
//-----------------------------------------------------------------------------
CCommandBase *CCommandBase::FindCommandPart( char const *name, std::vector<CCommandBase*> &ret )
{
	CCommandBase *cmd = GetCommands();
	Assert(cmd);
	for ( ; cmd; cmd = cmd->GetNext() )
	{
		if ( !strncmp( name, cmd->GetName(), strlen(name) ) )
		{
			ret.push_back( cmd );
		}
		//return cmd;
	}
	return NULL;
}



//-----------------------------------------------------------------------------
// uniwersalna funkcja tworzenia cvar i ccommand
// dodaje zmienna na liste
//-----------------------------------------------------------------------------
void CCommandBase::Create( char const *pName, char const *pHelp /*= 0*/, int flags /*= 0*/ )
{
	m_pNext = s_pBase;
	s_pBase = this;

	m_pszName = pName;

	// Zapamietujemy dana komende/zmienna
	CCommandMgr::AddCommand( this );
}


//-----------------------------------------------------------------------------
// policz ilosc zadeklarowanych cvars w tym dllu
//-----------------------------------------------------------------------------
int CCommandBase::CountCVars( void )
{
	CCommandBase *next = s_pBase;
	int counter = 0;

	while (next)
	{
		next = next->m_pNext;
		++counter;
	}
	return counter;
}





CCommand::CCommand( char const *pName, FnCommandFunc func, char const *pHelp/* = 0*/, int flags/* = 0*/ )
{
	Create( pName, func, pHelp, flags );
}

//-----------------------------------------------------------------------------
// tworzenie nowej komendy oraz dodanie jej do listy
//-----------------------------------------------------------------------------
void CCommand::Create( char const *pName, FnCommandFunc func, char const *pHelp/* = 0*/, int flags/* = 0*/ )
{
	// przypiszmy funckje do komedy
	m_fnCommandFunc = func;

	// uruchommy podstawowy Create aby dodac do listy
	BaseClass::Create(pName,pHelp,flags);
}

//-----------------------------------------------------------------------------
// odpal przypisana funkcje
//-----------------------------------------------------------------------------
void CCommand::Dispatch()
{
	if (m_fnCommandFunc)
	{
		(*m_fnCommandFunc)();
	} else
		Assert(0);//"Tried to dispatch not assigned command function!");
}



CCommand::~CCommand()
{
}

bool CCommand::IsCommand()
{
	return true;
}


//-----------------------------------------------------------------------------
// pelna lista konstruktorow dla CVar
//-----------------------------------------------------------------------------
CVar::CVar(char const *pName, char const *pDefaultValue, int flags/* = 0*/)
{
	Create( pName, pDefaultValue, flags );	
}

CVar::CVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelp )
{
	Create( pName, pDefaultValue, flags, pHelp );	
}

CVar::CVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelp, bool bMin, float fMin, bool bMax, float fMax )
{
	Create( pName, pDefaultValue, flags, pHelp, bMin, fMin, bMax, fMax );	
}

CVar::CVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelp, FnChangeFunc func )
{
	Create( pName, pDefaultValue, flags, pHelp, false, 0.0, false, 0.0, func );	
}

CVar::CVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelp, bool bMin, float fMin, bool bMax, float fMax, FnChangeFunc func )
{
	Create( pName, pDefaultValue, flags, pHelp, bMin, fMin, bMax, fMax, func );
}


//-----------------------------------------------------------------------------
// tworzenie nowej zmiennej oraz dodanie jej do listy
//-----------------------------------------------------------------------------
void CVar::Create( char const *pName, char const *pDefaultValue, int flags /*= 0*/,
				  char const *pHelp /*= 0*/, bool bMin /*= false*/, float fMin /*= 0.0*/,
				  bool bMax /*= false*/, float fMax /*= false*/, FnChangeFunc func /*= 0*/ )
{
	static char *empty_string = "";

	// jesli nie podano wartosci domyslnej, musimy tak czy inaczej stworzyc poprawny lancuch
	m_pszDefaultValue	= pDefaultValue ? pDefaultValue : empty_string;
	Assert( pDefaultValue );	

	m_StringLength = strlen( m_pszDefaultValue ) + 1;
	memcpy( m_pszString, m_pszDefaultValue, m_StringLength );

	m_bHasMin = bMin;
	m_fMinVal = fMin;
	m_bHasMax = bMax;
	m_fMaxVal = fMax;

	m_iFlags = flags;

	m_fnChangeFunc = func;

	m_fValue = ( float )atof( m_pszString );

	if ( m_bHasMin && ( m_fValue < m_fMinVal ) )
	{
		Assert( 0 );
	}

	if ( m_bHasMax && ( m_fValue > m_fMaxVal ) )
	{
		Assert( 0 );
	}

	m_iValue = ( int )m_fValue;

	BaseClass::Create( pName, pHelp, flags );
}

//-----------------------------------------------------------------------------
// zbior funkcji get
//-----------------------------------------------------------------------------
float CVar::GetFloat()
{
	if (IsCommand())
		Assert(0);
	return m_fValue;
}

int CVar::GetInt()
{
	if (IsCommand())
		Assert(0);
	return m_iValue;
}

char const *CVar::GetString()
{
	return ( m_pszString ) ? m_pszString : "";
}


//-----------------------------------------------------------------------------
// dostosuj wartosc do ograniczen
//-----------------------------------------------------------------------------
bool CVar::ClampValue( float& value )
{
	if ( m_bHasMin && ( value < m_fMinVal ) )
	{
		value = m_fMinVal;

		return 1;
	}

	if ( m_bHasMax && ( value > m_fMaxVal ) )
	{
		value = m_fMaxVal;
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// ustaw wartosc
//-----------------------------------------------------------------------------
bool CVar::SetValue( int value )
{
	float fValue = (float)value;

	// sprawdzmy limity
	if (ClampValue(fValue))
	{
		value = (int)fValue;
	}

	m_iValue = value;
	m_fValue = fValue;

	sprintf( m_pszString, "%d", value );

	return true;
}

bool CVar::SetValue( float value )
{
	// sprawdzmy limity
	ClampValue(value);

	m_iValue = (int)value;
	m_fValue = value;

	sprintf( m_pszString, "%f", value );

	return true;
}

bool CVar::SetValue( char const *value )
{
	float fValue = (float)atof(value);

	float before = fValue;

	ClampValue(fValue);

	m_iValue = (int)fValue;
	m_fValue = fValue;

	if ( m_bHasMin || m_bHasMax )
		sprintf( m_pszString, "%d", m_iValue );
	else
		sprintf( m_pszString, "%s", value );

	return true;
}

//-----------------------------------------------------------------------------
// ustawia wartosc domyslna
//-----------------------------------------------------------------------------
void CVar::Reset()
{
	SetValue(m_pszDefaultValue);
}

//-----------------------------------------------------------------------------
// resetuje wszystkie zmienne w obrebie dll'a
//-----------------------------------------------------------------------------
void CVar::ResetAll()
{
	CCommandBase *p = s_pBase;
	while (p)
	{
		((CVar*)p)->Reset();
		p = GetNext();
	}
}

bool CVar::GetMin( float& value )
{
	value = m_fMinVal;
	return m_bHasMin;
}

bool CVar::GetMax( float& value )
{
	value = m_fMaxVal;
	return m_bHasMax;
}

CVar::~CVar()
{
}

bool CVar::IsCommand()
{
	return false;
}
