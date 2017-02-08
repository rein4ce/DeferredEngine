#include "platform.h"
#include "Console.h"
#include "CVar.h"

CConsole gConsole;

//////////////////////////////////////////////////////////////////////////
void CmdCommandList_f()
{
	gConsole.Output("Full command list:");
	for (int i=0;i<gConsole.m_CommandList.Size();i++)
	{
		if ( gConsole.m_CommandList[i]->IsCommand() )
		{
			gConsole.Output("    %s", gConsole.m_CommandList[i]->GetName() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CmdCvarList_f()
{
	gConsole.Output("Full CVar list:");
	for (int i=0;i<gConsole.m_CommandList.Size();i++)
	{
		if ( !gConsole.m_CommandList[i]->IsCommand() )
		{
			gConsole.Output("    %s = %s", ((CVar*)gConsole.m_CommandList[i])->GetName(), ((CVar*)gConsole.m_CommandList[i])->GetString() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CmdExec_f()
{
	if ( gConsole.GetArgumentCount() < 2 )
		return;

	std::ifstream file( gConsole.GetArgument(1), ios::in );

	if ( !file.is_open() )
	{
		gConsole.Output("Unable to exec '%s'", gConsole.GetArgument(1) );
		return;
	}

	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	gConsole.AddCommand("\n");
	gConsole.AddCommand( (char*)str.c_str());
	gConsole.AddCommand("\n");
	
	file.close();
}

//////////////////////////////////////////////////////////////////////////
void CmdEcho_f()
{
	if ( gConsole.GetArgumentCount() < 2 )
		return;
	gConsole.Output( gConsole.GetArgument(1) );
}

//////////////////////////////////////////////////////////////////////////
CCommand cmd_commandlist	( "cmdlist",	CmdCommandList_f );
CCommand cmd_cvarlist		( "cvarlist",	CmdCvarList_f );
CCommand cmd_exec			( "exec",		CmdExec_f );
CCommand cmd_echo			( "echo",		CmdEcho_f );

//////////////////////////////////////////////////////////////////////////
CConsole::CConsole()
{
	memset( &m_CmdText, 0, sizeof( m_CmdText ) );
	m_CmdText.data		= (unsigned char *)m_szCmdBuffer;
	m_CmdText.maxsize	= sizeof( m_szCmdBuffer );
	m_szCmdNullString	= "";
	m_szCmdArgs			= NULL;
	m_iCmdArgc			= 0;
	m_fnOutputFunc		= NULL;
}

//////////////////////////////////////////////////////////////////////////
CConsole::~CConsole()
{
}

//////////////////////////////////////////////////////////////////////////
void CConsole::AddCommand(char *sCommand)
{
	int 	l;

	l = strlen (sCommand);

	// jesli overflow, napisz w konsoli
	if (m_CmdText.cursize + l >= m_CmdText.maxsize)
	{
		return;
	}
	
	// dodaj na koniec
	SZ_Write( &m_CmdText, sCommand, strlen (sCommand) );
}

//////////////////////////////////////////////////////////////////////////
void CConsole::AddCommandToHead(char *sCommand)
{
	char	*temp;
	int	templen;

	// skopiuj wszystkie czekajace komendy
	templen = m_CmdText.cursize;
	if ( templen )
	{
		temp = (char *)new char[ templen ];
		memcpy( temp, m_CmdText.data, templen );
		SZ_Clear( &m_CmdText );
	}
	else
		temp = NULL;

	// najpier dodajemy zadana komende
	AddCommand( sCommand );

	// a potem skopiowana reszte
	if (templen)
	{
		SZ_Write( &m_CmdText, temp, templen );
		delete[] temp;
	}	
}

//////////////////////////////////////////////////////////////////////////
void CConsole::Execute()
{
	int	i;
	char	*text;
	char	line[1024];
	int	quotes;

	while (m_CmdText.cursize)
	{
		// find a  or ; line break
		text = (char *)m_CmdText.data;

		quotes = 0;
		for (i=0 ; i< m_CmdText.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}


		memcpy( line, text, i );
		line[i] = 0;

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if (i == m_CmdText.cursize)
			m_CmdText.cursize = 0;
		else
		{
			i++;
			m_CmdText.cursize -= i;
			memcpy (text, text+i, m_CmdText.cursize);
		}

		// execute the command line
		ExecuteString (line);

		// TODO: nagrywanie komend
	}
}

//////////////////////////////////////////////////////////////////////////
void CConsole::ExecuteString(char *sCommand)
{
	// przekaz aktualne zrodlo wywolania
	//g_CmdSource = src;

	// podziel linijke na slowa. od teraz mamy do nich dostep przez funkcje dostepowe
	TokenizeString (sCommand);

	// jesli nie ma zadnych slow, koniec
	if (!m_iCmdArgc)
		return;

	// jesli to komentarz, pomin
	if ( strlen(sCommand)>1 && sCommand[0]=='/' && sCommand[1]=='/' )
		return;

	// Sprawdzamy czy istnieje dana komenda lub zmienna
	CCommandBase *pCommand = GetCommand( GetArgument(0) );
	if ( !pCommand )
	{
		Output( "Unknown command: %s", GetArgument(0) );
		return;
	}

	// Jesli to komenda, wykonujemy ja i konczymy
	if ( pCommand->IsCommand() )
	{
		(( CCommand * )pCommand )->Dispatch();
		return;
	}

	// Jesli to zmienna:
	// Jesli jeden wyraz, pokaz zmienna
	if ( m_iCmdArgc == 1 )
	{
		Output("    %s = %s", (( CVar * )pCommand)->GetName(), (( CVar * )pCommand)->GetString() );
		return;
	}

	// Jesli dwa wyrazy, sprobuj przypisac wartosc
	if ( m_iCmdArgc == 2 )
	{
		CVar *cv = ( CVar * )pCommand;

		cv->SetValue( GetArgument(1) );

		Output("CVar changed:   %s  =  %s", GetArgument(0), cv->GetString());
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CConsole::TokenizeString(char *text)
{
	int		i;

	// usun argumenty z poprzedniego wywolania
	for (i=0 ; i<m_iCmdArgc ; i++)
	{
		delete[] m_szCmdArgv[i];
	}

	m_iCmdArgc = 0;
	m_szCmdArgs = NULL;

	while (1)
	{
		// pomin wszystkie spacje, ostatecznie 
		while (*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}

		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		// jesli dalej juz nic nie ma
		if (!*text)
			return;

		// jesli komende juz mamy zapisana, reszte przydziel jako parametr
		if (m_iCmdArgc == 1)
			m_szCmdArgs = text;

		// pobierz nastepne slowo
		char token[256];
		text = COM_Parse (text,token);


		// jesli nie ma juz nic wiecej, koniec
		if (!text)
			return;

		// dodaj kolejne slowo do listy argumentow
		if (m_iCmdArgc < MAX_ARGS)
		{
			m_szCmdArgv[m_iCmdArgc] = (char *)new char[ strlen(token)+1 ];
			strcpy (m_szCmdArgv[m_iCmdArgc], token);
			m_iCmdArgc++;
		}
	}

}

//////////////////////////////////////////////////////////////////////////
void CConsole::SetOutputFunc( FnOutputPrint func )
{
	m_fnOutputFunc = func;
}

//////////////////////////////////////////////////////////////////////////
void CConsole::Output( char *sText, ... )
{
	if ( !m_fnOutputFunc )
		return;

	char pBuffer[1024];
	va_list args;
	va_start( args, sText );
	vsprintf( &pBuffer[0], sText, args );
	m_fnOutputFunc( pBuffer );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
char *CConsole::GetArgument( int iArg )
{
	if ( (unsigned)iArg >= (unsigned)m_iCmdArgc )
		return m_szCmdNullString;
	return m_szCmdArgv[iArg];	
}

//////////////////////////////////////////////////////////////////////////
int CConsole::GetArgumentCount( void )
{
	return m_iCmdArgc;
}

//////////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar( CVar *pVar )
{
	m_CommandList.AddToTail( (CCommandBase*)pVar );
}

//////////////////////////////////////////////////////////////////////////
void CConsole::RegisterCommand( CCommand *pCommand )
{
	m_CommandList.AddToTail( (CCommandBase*)pCommand );
}

//////////////////////////////////////////////////////////////////////////
void CConsole::Init( CArray<CCommandBase*> *pList )
{
	m_CommandList.AddVectorToTail( *pList );
}

//////////////////////////////////////////////////////////////////////////
CVar* CConsole::GetCVar( const char *pName )
{
	for (int i=0;i<m_CommandList.Size();i++)
	{
		if ( strcmp(m_CommandList[i]->GetName(),pName)==0 )
			return (CVar*)m_CommandList[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CCommand* CConsole::GetCommand( const char *pName )
{
	for (int i=0;i<m_CommandList.Size();i++)
	{
		if ( strcmp(m_CommandList[i]->GetName(),pName)==0 )
			return (CCommand*)m_CommandList[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CConsole::WriteVariables( void )
{
	CVar* cv;

	std::ofstream file( "config//config.cfg" ,ios::out );
	file.clear();

	for (int i=0;i<m_CommandList.Size();i++)
	{
		if ( !m_CommandList[i]->IsFlagSet( FCVAR_ARCHIVE ) )
			continue;

		cv = (CVar*)m_CommandList[i];
		file << cv->GetName();
		file << " ";
		file << cv->GetString() << "\n";
	}
	
	file.close();
}