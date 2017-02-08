#include "platform.h"
#include "Common.h"





//////////////////////////////////////////////////////////////////////////
//
//	SIZEBUFFERS
//
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// funkcje do obslugi sizebuf_t
//-----------------------------------------------------------------------------
void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

//-----------------------------------------------------------------------------
// zwraca wskaznik do wolnego miejsca w bufferze
//-----------------------------------------------------------------------------
void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void    *data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
		{
			if (!buf->maxsize)
				Error ("SZ_GetSpace: sizebuf_t not ready to write");
			else
				Error ("SZ_GetSpace: overflow without allowoverflow set");
		}
		else
		{
			//			Con_Printf( "overflowing, ok\n" );
		}

		if (length > buf->maxsize)
			Error ("SZ_GetSpace: %i is > full buffer size", length);

		//		Con_Printf ("SZ_GetSpace: overflow\n");
		SZ_Clear (buf); 
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

//-----------------------------------------------------------------------------
// wpisz dane do buffera w pierwsze wolne miejsce
//-----------------------------------------------------------------------------
void SZ_Write (sizebuf_t *buf, const void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);         
}

inline void SZ_Print (sizebuf_t *buf, const char *data)
{
	int             len;

	len = strlen(data)+1;

	if (buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}

//////////////////////////////////////////////////////////////////////////
void Tokenize(const string& str,
			  vector<string>& tokens,
			  const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}