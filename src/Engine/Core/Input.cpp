#include "platform.h"
#include "Input.h"
#include "Common.h"
#include "Console.h"
#include "Engine.h"
#include "CVar.h"
#include "ConsolePanel.h"

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

CInput			gInput;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},


	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },
	{"CAPSLOCK",		K_CAPSLOCK },

	{"MWHEELUP", K_MWHEELUP },
	{"MWHEELDOWN", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

const byte g_iTypeChars[2]	= { 33, 126 };
const byte g_iDigits[2]		= { 48, 57 }; 
const byte g_iCapitals[2]	= { 65, 90 };
const byte g_iSmall[2]		= { 97, 122 };

//////////////////////////////////////////////////////////////////////////
void Cmd_Bind_f(void)
{
	int c = gConsole.GetArgumentCount();
	int b;

	if ( c != 3 )
	{
		gConsole.Output("Usage: bind <key> [command]");
		return;
	} 

	b = gInput.StringToKeynum(gConsole.GetArgument(1));
	if (b==-1)
	{
		gConsole.Output("\"%s\" isn't a valid key\n", gConsole.GetArgument(1));
		return;
	}
	gInput.BindKey(b,gConsole.GetArgument(2));
}

CCommand cmd_bind		( "bind",		Cmd_Bind_f );

//////////////////////////////////////////////////////////////////////////
CInput::CInput()
{
	memset( m_iKeyStates, 0, 256 );
	memset( m_iOldKeyStates, 0, 256 );
	memset( m_iKeyRepeats, 0, 256 );
	memset( m_pszKeyBinds, 0, 256 );
	memset( m_iKeyPressed, 0, 256 );
	memset( m_iKeyReleased, 0, 256 );

	m_iKeyLast = -1;
	m_iKeysPressed = 0;
	m_iMouseOldButtons = 0;
	m_bCenterCursor = false;
	GetMousePos( m_iMouseX, m_iMouseY );
	m_iOldMouseX = m_iMouseX;
	m_iOldMouseY = m_iMouseY;
	m_iDeltaX = m_iDeltaY = 0;

	m_iWindowX = 0;
	m_iWindowY = 0;
	m_iWindowWidth = 0;
	m_iWindowHeight = 0;

	m_bTypingMode = false;
	m_fnTypingEnterFunc = NULL;
	firstFrame = true;
}

CInput::~CInput()
{
}

void CInput::Release()
{
	UnbindAll();
}

//////////////////////////////////////////////////////////////////////////
void CInput::KeyEvent( int key, bool down )
{
	if ( m_iKeyStates[key] == false && 
		down == false )
	{
		return;
	}

	m_iKeyStates[key] = down;

	if ( down )
	{
		// jesli to nowy przycisk, zwieksz ilosc wcisnietych
		if ( m_iKeyRepeats[key]==0 ) m_iKeysPressed++;
		
		// oznacz przycisk jako nacisniety
		m_iKeyPressed[key] = 1;

		m_iKeyRepeats[key]++;		// zwieksz ilosc powtorzen
		if ( key != K_BACKSPACE &&
			key != K_PAUSE &&
			key != K_PGUP &&
			key != K_KP_PGUP &&
			key != K_PGDN && 
			key != K_KP_PGDN && 
			m_iKeyRepeats[key] > 1)
		{
			return;
		}
	} else
	{
		m_iKeyReleased[key] = 1;
		m_iKeyRepeats[key] = 0;		// wyzeruj powtorzenia
		m_iKeysPressed--;
	}

	if ( m_iKeyLast != key )
		m_iKeyLast = key;

	// jesli to tryb pisania, uniewaznij zbindowane przyciski (za wyjatkiem ESC i tyldy)
	if ( m_bTypingMode && key!=K_ESCAPE && key!='`' )
	{
		char k = TypeKeyPressed();

		// Jesli to litera, dodaj na koniec
		if ( k )
		{
			m_strTypingText.push_back( k );
		} else
		{
			if ( WasKeyPressed( K_SPACE ) )
				m_strTypingText.push_back( ' ' );

			if ( WasKeyPressed( K_BACKSPACE ) && m_strTypingText.size()>0 )
				m_strTypingText.resize( m_strTypingText.size()-1 );

			if ( WasKeyPressed( K_ENTER ) )
			{
				if ( m_fnTypingEnterFunc )
					m_fnTypingEnterFunc();
			}
		}
	} 
	else
	{
		// zinterpretuj przycisk, sprawdz czy nie ma przypisanej komendy
		if (  down && m_pszKeyBinds[key] )
		{
			gConsole.AddCommand( m_pszKeyBinds[key] );
			gConsole.AddCommand("\n");
		} 

		// jesli jednak zwalniamy przycisk, sprawdz czy przypadkiem komenda nie ma "+" na poczatku
		// w takim przypadku przy zwalnianiu przycisku zostaje wywoalana komenda z "-"
		if (  !down && m_pszKeyBinds[key] && m_pszKeyBinds[key][0] == '+' )
		{
			char cmd[128];
			sprintf( cmd, "-%s", m_pszKeyBinds[key]+1 );
			gConsole.AddCommand( cmd );
			gConsole.AddCommand("\n");

			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::MouseEvent( int state, bool down )
{
	m_iMouseButtons = state;

	for ( int i=0; i<3; i++ )
	{
		if ( (state & (1<<i)) && !(m_iMouseOldButtons & (1<<i)) )
		{
			KeyEvent(K_MOUSE1 + i, down);

		} else
			if ( !(state & (1<<i)) && (m_iMouseOldButtons & (1<<i)) )
			{
				KeyEvent(K_MOUSE1 + i, 0);
			}
	}

	if ( state != m_iMouseOldButtons )
		m_iMouseOldButtons = state;	
}

//////////////////////////////////////////////////////////////////////////

int CInput::StringToKeynum (char *str)
{
	keyname_t	*kn;

	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

	for (kn=keynames ; kn->name ; kn++)
	{
		if (!Q_strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

char *CInput::KeynumToString(int keynum)
{
	keyname_t	*kn;	
	static	char	tinystr[2];

	if (keynum == -1)
		return "<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn=keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}

//////////////////////////////////////////////////////////////////////////

char CInput::IsKeydown( int key )
{
	return m_iKeyStates[key];
}

bool CInput::WasKeyPressed( int key )
{
	return m_iKeyPressed[key];
}

bool CInput::WasKeyReleased( int key )
{
	return m_iKeyReleased[key];
}

byte CInput::TypeKeyPressed(void)
{
	if ( WasKeyPressed(m_iKeyLast) )
	{
		int key = m_iKeyLast;
		if (m_iKeyLast>=g_iTypeChars[0] && m_iKeyLast<=g_iTypeChars[1] )
		{
			// jesli SHIFT
			if ( IsKeydown( K_SHIFT ) )
			{
				if ( IsKeySmall(key) )	key-=32; else
				if ( key>90 && key<96 ) key+=32; else
				
				if ( key == ';' )	key=':'; else
				if ( key == 39 )	key='"'; else
				if ( key == '.' )	key='>'; else
				if ( key == '/' )	key='?'; else
				if ( key == ',' )	key='<'; else
				if ( key == '=' )	key='+'; else
				if ( key == '-' )	key='_'; else
				if ( key == 39 )	key='"'; else
				if ( key == 50 )	key=64; else	// 2
				if ( key == 54 )	key=94; else	// 6
				if ( key == 55 )	key=38; else	// 7
				if ( key == 56 )	key=42; else	// 8
				if ( key == 57 )	key=40; else	// 9
				if ( key == 48 )	key=41; else	// 0
				if ( key < g_iCapitals[0] )	key-=16;
				
			}

			return (byte)key;
		}
	}
	return 0;
}

bool	CInput::IsKeyTypeKey( int key )
{
	return (key>=g_iTypeChars[0] && key<=g_iTypeChars[1]);
}

bool	CInput::IsKeyDigit( int key )
{
	return (key>=g_iDigits[0] && key<=g_iDigits[1]);
}

bool	CInput::IsKeyCapital( int key )
{
	return (key>=g_iCapitals[0] && key<=g_iCapitals[1]);
}

bool	CInput::IsKeySmall( int key )
{
	return (key>=g_iSmall[0] && key<=g_iSmall[1]);
}

int CInput::KeysPressedNum( void )
{
	return m_iKeysPressed;
}

int CInput::MouseState( void )
{
	return m_iMouseButtons;
}

//////////////////////////////////////////////////////////////////////////
void CInput::Frame()
{
	m_iOldMouseX = m_iMouseX;
	m_iOldMouseY = m_iMouseY;
	GetMousePos( m_iMouseX, m_iMouseY );

	if (firstFrame)
	{
		m_iDeltaX = m_iDeltaY = 0;
		firstFrame = false;
	}
	else
	{
		m_iDeltaX = m_iMouseX - m_iOldMouseX;
		m_iDeltaY = m_iMouseY - m_iOldMouseY;
	}
	
	// center the cursor if window has focus and option is set
	if ( m_bCenterCursor && gEngine.HasFocus() ) 
	{
		SetCursorPos( m_iWindowX + m_iWindowWidth/2, m_iWindowY + m_iWindowHeight/2 );
		GetMousePos( m_iMouseX, m_iMouseY );
	}

	// resetuj liste wcisnietych i zwolnionych przyciskow
	for ( int i=0; i<256; i++ )
	{
		m_iKeyPressed[i] = 0;
		m_iKeyReleased[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
int CInput::GetMouseX()
{
	return m_iMouseX;
}

int CInput::GetMouseY()
{
	return m_iMouseY;
}

//////////////////////////////////////////////////////////////////////////

void CInput::BindKey( int key, char *command )
{
	if ( key == -1 )
		return;

	// usun poprzedni wpis
	if ( m_pszKeyBinds[key] )
	{
		// jesli ta sama komenda, nie re-binduj
		if ( !strcmp( m_pszKeyBinds[key], command ) )
			return;

		// usun poprzedni wpis
		delete[] m_pszKeyBinds[key];
		m_pszKeyBinds[key] = NULL;
	}

	// przypisz nowy
	int l = strlen( command );
	char *newbind = (char *)new char[l+1];
	strcpy( newbind, command );
	newbind[l] = 0;
	m_pszKeyBinds[key] = newbind;
}

void CInput::UnbindAll()
{
	for (int i=0; i<256; i++)
	{
		if ( m_pszKeyBinds[i]!=NULL )
			delete[] m_pszKeyBinds[i];

		m_pszKeyBinds[i] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::DefaultWindowUpdate( HWND hWnd )
{
	POINT clientPos;
	clientPos.x = 0;
	clientPos.y = 0;
	ClientToScreen(hWnd, &clientPos);
	
	m_iWindowX = clientPos.x;
	m_iWindowY = clientPos.y;
}

void CInput::SetWindowData( int x, int y, int w, int h )
{
	m_iWindowX = x;
	m_iWindowY = y;
	m_iWindowWidth = w;
	m_iWindowHeight = h;
}

void CInput::GetAbsoluteMousePos( int &x, int &y )
{
	POINT point;
	GetCursorPos( &point );

	x = point.x;
	y = point.y;
}

bool CInput::IsMouseInRect(int x, int y, int width, int height)
{
	int mx, my;
	GetMousePos(mx, my);
	return mx >= x && my >= y && mx <= x+width && my <= y+height;
}

void CInput::GetMousePos( int &x, int &y )
{
	POINT point;
	GetCursorPos( &point );

	x = point.x - m_iWindowX;
	y = point.y - m_iWindowY;

	x = max( min( m_iWindowWidth, x) , 0 );
	y = max( min( m_iWindowHeight, y) , 0 );
}

//////////////////////////////////////////////////////////////////////////
void CInput::BeginTyping(const char *pStartingText, FnEnterFunc funcEnter, int iFlags)
{
	m_bTypingMode = true;
	m_fnTypingEnterFunc = funcEnter;
	m_iTypingFlags = iFlags;

	m_strTypingText.clear();
	m_strTypingText.assign( pStartingText );
}

//////////////////////////////////////////////////////////////////////////
void CInput::EndTyping()
{
	m_bTypingMode = false;
}

//////////////////////////////////////////////////////////////////////////
const char *CInput::GetTypingText()
{
	return m_strTypingText.c_str();
}

//////////////////////////////////////////////////////////////////////////
void CInput::SetCenterCursor( bool center )
{
	m_bCenterCursor = center;
}