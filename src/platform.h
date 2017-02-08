#pragma once

#include <windows.h>
#include <vector>
#include <list>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <fstream>
#include <time.h>
#include <map>
#include <sstream>
#include <map>
#include <algorithm>
#include <stack>
#include <streambuf>
#include <queue>
#include <string>
#include <streambuf>

using namespace std;

#include <boost/thread/condition_variable.hpp>
#include <boost/shared_ptr.hpp>

#include <D3D10.h>
#include <D3DX10.h>
#include <DXGI.h>
#include <D3DX10math.h>






class Direct3D;
extern Direct3D *gD3D;

#ifndef OUT
#define OUT
#endif


//////////////////////////////////////////////////////////////////////////

typedef LRESULT (WINAPI *FnWndProc )( HWND, UINT, WPARAM, LPARAM );

//////////////////////////////////////////////////////////////////////////

typedef signed char			sint8;
typedef unsigned char		uint8;
typedef signed short		sint16;
typedef unsigned short		uint16;
typedef signed int			sint32;
typedef unsigned int		uint32;
typedef signed long			sint64;
typedef unsigned long		uint64;
typedef unsigned char		byte;

//////////////////////////////////////////////////////////////////////////
inline void trace(string s)
{
	OutputDebugString(s.c_str());
	OutputDebugString("\n");
}


inline void trace(char *msg, ...)
{
	char pBuffer[1024];
	va_list args;
	va_start( args, msg );
	vsprintf( &pBuffer[0], msg, args );
	OutputDebugString(pBuffer);
	OutputDebugString("\n");
	va_end(args);
}

inline void Debug(char *msg, ...)
{
	char pBuffer[1024];
	va_list args;
	va_start( args, msg );
	vsprintf( &pBuffer[0], msg, args );
	OutputDebugString(pBuffer);
	OutputDebugString("\n");
	va_end(args);
}

#define Singleton(className) \
	private: className(); \
	public: static className* Instance() { static className instance; return &instance; } \



using namespace std;			// szybki dostep do funkcji STD


#ifdef _WIN32
// Remove warnings from warning level 4.
#pragma warning(disable : 4514) // warning C4514: 'acosl' : unreferenced inline function has been removed
#pragma warning(disable : 4100) // warning C4100: 'hwnd' : unreferenced formal parameter
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4512) // warning C4512: 'InFileRIFF' : assignment operator could not be generated
#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4706) // warning C4706: assignment within conditional expression
#pragma warning(disable : 4710) // warning C4710: function 'x' not inlined
#pragma warning(disable : 4702) // warning C4702: unreachable code
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4239) // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
#pragma warning(disable : 4097) // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
#pragma warning(disable : 4324) // Padding was added at the end of a structure
#pragma warning(disable : 4244) // type conversion warning.
#pragma warning(disable : 4305)	// truncation from 'const double ' to 'float '
#pragma warning(disable : 4786)	// Disable warnings about long symbol names


#if _MSC_VER >= 1300
#pragma warning(disable : 4511)	// Disable warnings about private copy constructors
#endif
#endif

//////////////////////////////////////////////////////////////////////////

const float ToRad = 3.14159f/180.0f;
const float ToDeg = 180.0f/3.14159f;

#define DEG2RAD(a)  ((a)*ToRad)
#define RAD2DEG(a)  ((a)*ToDeg)

// RTTI function generator
#define TYPE(a) public: virtual string	GetType() { return a; };


inline short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

inline short	ShortNoSwap (short l)
{
	return l;
}

inline int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

inline int	LongNoSwap (int l)
{
	return l;
}

typedef union 
{
	float	f;
	unsigned int i;
} _FloatByteUnion;

inline float FloatSwap (const float *f) {
	const _FloatByteUnion *in;
	_FloatByteUnion out;

	in = (_FloatByteUnion *)f;
	out.i = LongSwap(in->i);

	return out.f;
}

inline float FloatNoSwap (const float *f)
{
	return *f;
}

#define ID_INLINE __inline 

static ID_INLINE short BigShort( short l) { return ShortSwap(l); }
#define LittleShort
static ID_INLINE int BigLong(int l) { LongSwap(l); }
#define LittleLong
static ID_INLINE float BigFloat(const float *l) { FloatSwap(l); }
#define LittleFloat

#define	PATH_SEP '\\'

#if defined(DEBUG) || defined(_DEBUG)
#ifndef DXV
#define DXV(x)           { HRESULT hr = (x); if( FAILED(hr) ) { Error( "Error occurred:\nFile:\t %s\nLine:\t %d\nHRESULT:\t %d", __FILE__, (DWORD)__LINE__, hr, L#x, true ); exit(0); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { HRESULT hr = (x); if( FAILED(hr) ) { trace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); return hr; } }
#endif
#else
#ifndef DXV
#define DXV(x)           { HRESULT hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { HRESULT hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete [] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE_DELETE
#define SAFE_RELEASE_DELETE(p)			{ if(p) { (p)->Release();	delete p; (p)=NULL; } }
#endif


#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)


//////////////////////////////////////////////////////////////////////////
// DEBUG
//////////////////////////////////////////////////////////////////////////

// jesli chcemy porzucic info i LINE i FILE, uzyjmy tego przy podawaniu line do DebugMessage
#define DISCARD_ASSERTION_LOCATION		-1

enum DebugType_t
{
	DEBUG_MESSAGE	= 0,
	DEBUG_ASSERT,
	DEBUG_ERROR,
	DEBUG_LOG
};

enum DebugRetVal_t
{
	DBGRET_CONTINUE	= 0,
	DBGRET_ABORT,
	DBGRET_DEBUGGER
};


#ifndef _DEBUG
#define _DEBUG
#endif

// funkcja powodujaca zatrzymanie debuggera
#if defined (_DEBUG)
#define DebuggerBreak()		{ __asm int 3 }
#else
#define DebuggerBreak()
#endif

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////
// Funkcje Assert
//////////////////////////////////////////////////////////////////////////

#define Assert( _exp )			if (!(_exp))		\
{			\
	DebuggerBreak();										\
}													\
															


#else

#define Assert( _exp )		
	

#endif

//////////////////////////////////////////////////////////////////////////
// Wyswietlanie komunikatow
//////////////////////////////////////////////////////////////////////////

// Krytyczny blad programu, wyswietla komunikat i zamyka aplikacje bez zwalniania pamieci
inline void Error( char const *pMsgFormat, ... )
{
	char pBuffer[1024];
#ifndef EDITOR_BUILD
	ShowCursor(1);
#endif
	va_list args;
	va_start( args, pMsgFormat );
	vsprintf( &pBuffer[0], pMsgFormat, args );
#ifndef EDITOR_BUILD
	MessageBox( NULL, pBuffer, "VoidError", MB_OK );
#else
	//MessageBox::Show( pBuffer );
#endif
	va_end(args);
#ifndef EDITOR_BUILD
	ShowCursor(0);
#endif
	exit(1);
}

inline void Message( char const *pMsgFormat, ... )
{
	char pBuffer[1024];
#ifndef EDITOR_BUILD
	ShowCursor(1);
#endif
	va_list args;
	va_start( args, pMsgFormat );
	vsprintf( &pBuffer[0], pMsgFormat, args );
#ifndef EDITOR_BUILD
	MessageBox( NULL, pBuffer, "VoidMessage", MB_OK );
#else
	//MessageBox::Show( pBuffer );
#endif

	va_end(args);
#ifndef EDITOR_BUILD
	ShowCursor(0);
#endif
}

inline void Log( char const *pMsgFormat, ... )
{
	va_list args;
	va_start( args, pMsgFormat );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
// Time
//////////////////////////////////////////////////////////////////////////

extern  LARGE_INTEGER g_PerformanceFrequency;
extern  LARGE_INTEGER g_MSPerformanceFrequency;
extern  LARGE_INTEGER g_ClockStart;

double GetFloatTime();


//////////////////////////////////////////////////////////////////////////

#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

// In case this ever changes
#define M_PI			3.14159265358979323846
#define M_PI_2			(M_PI/2.0f)
#define M_PI_4			(M_PI/4.0f)

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////////

inline double toRadians(double a)
{
	return (a*M_PI/180.0f);
}

inline void ClampRad(float &d)
{
	if (d<0) d = 2.0f*M_PI+d;
	if (d>2.0f*M_PI) d = d-2.0f*M_PI;	
}

inline void Clamp(int &i, int lower, int upper)
{
	i = min(upper,max(lower,i));
}

inline void Clamp(float &i, float lower, float upper)
{
	i = min(upper,max(lower,i));
}

#define clamp(i,lower,upper) min((upper),max((lower),(i)))

