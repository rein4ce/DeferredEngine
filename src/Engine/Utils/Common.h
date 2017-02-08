#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

// Typ funkcji do prezentowania tekstu (np. komunikaty o zmiennych)
typedef void (*FnOutputPrint)(char *);

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192	// used for system info key only
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192


//////////////////////////////////////////////////////////////////////////
//
//	UTILS
//
//////////////////////////////////////////////////////////////////////////

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

//////////////////////////////////////////////////////////////////////////
//
//	TYPY
//
//////////////////////////////////////////////////////////////////////////

typedef struct sizebuf_s
{
	bool	allowoverflow;	// if false, do a Sys_Error
	bool	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;



//////////////////////////////////////////////////////////////////////////
//
//	FUNKCJE STRING
//
//////////////////////////////////////////////////////////////////////////
inline int Q_snprintf( char *pDest, int maxLen, char const *pFormat, ... )
{
	va_list marker;

	va_start( marker, pFormat );
	int len = _vsnprintf( pDest, maxLen, pFormat, marker );
	va_end( marker );

	// Len < 0 represents an overflow
	if( len < 0 )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}

inline int Q_vsnprintf( char *pDest, int maxLen, char const *pFormat, va_list params )
{
	int len = _vsnprintf( pDest, maxLen, pFormat, params );

	if( len < 0 )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}

inline int Q_strncasecmp (const char *s1, const char *s2, int n)
{
	Assert( n >= 0 );
	if (!s1 || !s2 )
		return 1;

	int             c1, c2;

	while (1)
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;               // strings are equal until end point

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;              // strings not equal
		}
		if (!c1)
			return 0;               // strings are equal
		//              s1++;
		//              s2++;
	}

	return -1;
}

inline int Q_strcasecmp (const char *s1, const char *s2)
{
	if (!s1 || !s2 )
		return 1;
	
	return Q_strncasecmp (s1, s2, 99999);
}

inline int Q_strnicmp (const char *s1, const char *s2, int n)
{
	Assert( n >= 0 );
	if (!s1 || !s2 )
		return 1;

	return Q_strncasecmp( s1, s2, n );
}

inline int Q_atoi (const char *str)
{
	Assert(str);

	int             val;
	int             sign;
	int             c;

	Assert( str );
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}

	//
	// assume decimal
	//
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}

	return 0;
}


inline float Q_atof (const char *str)
{
	Assert(str);//AssertValidStringPtr( str );
	double			val;
	int             sign;
	int             c;
	int             decimal, total;

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val*16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val*16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val*16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}

	//
	// assume decimal
	//
	decimal = -1;
	total = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}

	return val*sign;
}


void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, const char *data);

inline void GetFileList (std::vector<std::string>& _list, std::string dir, std::string extension)
{

	WIN32_FIND_DATA findData;
	HANDLE fileHandle;
	int flag = 1;
	std::string search ("*.");
	if (dir == "") dir = ".";
	dir += "/";
	search = dir + search + extension;
	fileHandle = FindFirstFile(search.c_str(), &findData);
	if (fileHandle == INVALID_HANDLE_VALUE) return;
	while (flag)
	{
		_list.push_back(findData.cFileName);
		flag = FindNextFile(fileHandle, &findData);
	}
	FindClose(fileHandle);


}

inline void GetDirList (std::vector<std::string>& _list, std::string dir)
{
	WIN32_FIND_DATA findData;
	HANDLE fileHandle;
	int flag = 1;
	std::string search ("*");
	if (dir == "") dir = ".";
	dir += "/";
	search = dir + search;
	fileHandle = FindFirstFile(search.c_str(), &findData);
	if (fileHandle == INVALID_HANDLE_VALUE) return;
	while (flag)
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ( findData.cFileName[0]!='.' )
				_list.push_back(findData.cFileName);
		}
		flag = FindNextFile(fileHandle, &findData);
	}
	FindClose(fileHandle);
}


//-----------------------------------------------------------------------------
// parsuj dany lancuch. pierwsze pobrane slowo zapisz w g_szComToken
//-----------------------------------------------------------------------------
inline char *COM_Parse (char *data,char *token)
{
	int             c;
	int             len;
	char			g_szComToken[1024];

	len = 0;
	g_szComToken[0] = 0;

	if (!data)
		return NULL;

	// pomin wszystkie puste znaki
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;                    // end of file;
		data++;
	}

	// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}


	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				g_szComToken[len] = 0;
				strcpy(token,g_szComToken);
				return data;
			}
			g_szComToken[len] = c;
			len++;
		}
	}

	// kopiuj litera po literze, gdy znak o kodzie ponizej 32 napotkany, koniec
	// wynik wrzuc do g_szComToken
	do
	{
		g_szComToken[len] = c;
		data++;
		len++;
		c = *data;
	} while (c>32);

	g_szComToken[len] = 0;
	strcpy(token,g_szComToken);
	return data;
}

//////////////////////////////////////////////////////////////////////////
inline char	*va( char *format, ... ) {
	va_list		argptr;
	static char		string[2][32000];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;

	buf = string[index & 1];
	index++;

	va_start (argptr, format);
	vsprintf (buf, format,argptr);
	va_end (argptr);

	return buf;
}

//////////////////////////////////////////////////////////////////////////
void Tokenize(const string& str,
			  vector<string>& tokens,
			  const string& delimiters);

