#pragma once
#include "platform.h"
#include "keydefs.h"

typedef void (*FnEnterFunc)( void );

#define Keydown(a)			gInput.IsKeydown(a)
#define KeyPressed(a)		gInput.WasKeyPressed(a)
#define KeyReleased(a)		gInput.WasKeyReleased(a)

class CInput
{
public:
	CInput( void );
	~CInput( void );

	void	Release( void );
	void	Frame( void );

	int		GetMouseX();
	int		GetMouseY();
	inline	int	GetDeltaX() { return m_iDeltaX; }
	inline	int	GetDeltaY() { return m_iDeltaY; }

	void	KeyEvent( int key, bool down );				// Za pomoca tej funkcji informujemy klase o stanie przyciskow, moze byc pochodzic z petli Windowsa, moze skad inad
	void	MouseEvent( int state, bool down );			// Eventy myszy
	void	SetWindowData( int x, int y, int w, int h );// Uaktalnia dane okna, wzgledem ktorych liczymy wzgledne wspolrzedne kursora
	void	DefaultWindowUpdate( HWND hWnd );			// Uaktalnia dane okna za pomoca handlera
	void	SetCenterCursor( bool center );

	int		StringToKeynum( char *str );				// Nazwa przycisku zwraca indeks
	char	*KeynumToString( int keynum );				// Indeks przycisku zwraca jego nazwe

	char	IsKeydown( int key );						// zwraca czy wcisniety przycisk key
	int		KeysPressedNum( void );						// zwraca ilosc wcisnietych przyciskow
	bool	WasKeyPressed( int key );
	bool	WasKeyReleased( int key );
	byte	TypeKeyPressed(void);						// zwraca wcisniety przycisk type lub 0
	int		MouseState( void );							// zwraca stan przyciskow myszy

	void	GetMousePos( int &x, int &y );				// zwraca wspolrzedne myszy w/g okna
	void	GetAbsoluteMousePos( int &x, int &y );		// zwraca windowsowe koordynaty myszy
	bool	IsMouseInRect(int x, int y, int width, int height);

	bool	IsKeyTypeKey( int key );
	bool	IsKeyDigit( int key );
	bool	IsKeyCapital( int key );
	bool	IsKeySmall( int key );

	void	BeginTyping( const char *pStartingText, FnEnterFunc funcEnter = 0, int iFlags = 0 );// Rozpoczyna tryb pisania (anuluje bindy klawiszy)
	void	EndTyping( void );																	// Konczy tryb pisania
	const char	*GetTypingText( void );															// Zwraca napisany tekst

	void	BindKey( int key, char *command );			// przypisujemy id przycisku do komendy	np. BindKey( K_ESCAPE, "exit" )
	void	UnbindAll( void );							// zresetuj wszystkie przypisania

public:

	// Mysz
	int					m_iMouseX, m_iOldMouseX;
	int					m_iMouseY, m_iOldMouseY;
	int					m_iDeltaX;
	int					m_iDeltaY;
	int					m_iMouseButtons;
	bool				m_bMouseMoved;
	int					m_iMouseOldButtons;
	bool				m_bCenterCursor;

	int					m_iWindowX;					// dane okna, wzgledem ktorych zwracamy pozycje myszy
	int					m_iWindowY;
	int					m_iWindowWidth;
	int					m_iWindowHeight;

	// Klawiatura
	int					m_iKeyboardDelay;
	int					m_iKeyboardSpeed;
	unsigned int		m_iKeyLast;

	int					m_iKeysPressed;				// ilosc wlasnie wcisnietych klawiszy

	char				m_iKeyStates[256];			// aktualny stan klawiszy
	char				m_iOldKeyStates[256];
	int					m_iKeyRepeats[256];			// zlicza ilosc powtorzen
	bool				m_iKeyPressed[256];			// czy wlasnie nacisniety?
	bool				m_iKeyReleased[256];

	char				*m_pszKeyBinds[256];		// lista komend zbindowanych do kazdego przycisku

	// Typing
	bool				m_bTypingMode;
	int					m_iTypingFlags;
	FnEnterFunc			m_fnTypingEnterFunc;
	string				m_strTypingText;

private:
	bool				firstFrame;
};

extern CInput gInput;