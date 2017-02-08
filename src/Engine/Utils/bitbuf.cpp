#include "platform.h"
#include "bitbuf.h"
#include "common.h"


// FIXME: Can't use this until we get multithreaded allocations in tier0 working for tools
// This is used by VVIS and fails to link
// NOTE: This must be the last file included!!!
//#include "tier0/memdbgon.h"


static BitBufErrorHandler g_BitBufErrorHandler = 0;


void InternalBitBufErrorHandler( BitBufErrorType errorType, const char *pDebugName )
{
	if ( g_BitBufErrorHandler )
		g_BitBufErrorHandler( errorType, pDebugName );
}


void SetBitBufErrorHandler( BitBufErrorHandler fn )
{
	g_BitBufErrorHandler = fn;
}


// #define BB_PROFILING


// Precalculated bit masks for WriteUBitLong. Using these tables instead of 
// doing the calculations gives a 33% speedup in WriteUBitLong.
unsigned long g_BitWriteMasks[32][33];

// (1 << i) - 1
unsigned long g_ExtraMasks[32];

class CBitWriteMasksInit
{
public:
	CBitWriteMasksInit()
	{
		for( unsigned int startbit=0; startbit < 32; startbit++ )
		{
			for( unsigned int nBitsLeft=0; nBitsLeft < 33; nBitsLeft++ )
			{
				unsigned int endbit = startbit + nBitsLeft;
				g_BitWriteMasks[startbit][nBitsLeft] = (1 << startbit) - 1;
				if(endbit < 32)
					g_BitWriteMasks[startbit][nBitsLeft] |= ~((1 << endbit) - 1);
			}
		}

		for ( unsigned int maskBit=0; maskBit < 32; maskBit++ )
			g_ExtraMasks[maskBit] = (1 << maskBit) - 1;
	}
};
CBitWriteMasksInit g_BitWriteMasksInit;


// ---------------------------------------------------------------------------------------- //
// bf_write
// ---------------------------------------------------------------------------------------- //

bf_write::bf_write()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; // set to -1 so we generate overflow on any operation
	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

bf_write::bf_write( const char *pDebugName, void *pData, int nBytes, int nBits )
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartWriting( pData, nBytes, 0, nBits );
}

bf_write::bf_write( void *pData, int nBytes, int nBits )
{
	m_bAssertOnOverflow = true;
	StartWriting( pData, nBytes, 0, nBits );
}

void bf_write::StartWriting( void *pData, int nBytes, int iStartBit, int nBits )
{
	// Make sure it's dword aligned and padded.
	Assert( (nBytes % 4) == 0 );
	Assert(((unsigned long)pData & 3) == 0);

	m_pData = (unsigned char*)pData;
	m_nDataBytes = nBytes;

	if ( nBits == -1 )
	{
		m_nDataBits = nBytes << 3;
	}
	else
	{
		Assert( nBits <= nBytes*8 );
		m_nDataBits = nBits;
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

void bf_write::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}


void bf_write::SetAssertOnOverflow( bool bAssert )
{
	m_bAssertOnOverflow = bAssert;
}


const char* bf_write::GetDebugName()
{
	return m_pDebugName;
}


void bf_write::SetDebugName( const char *pDebugName )
{
	m_pDebugName = pDebugName;
}


void bf_write::SeekToBit( int bitPos )
{
	m_iCurBit = bitPos;
}


// Sign bit comes first
void bf_write::WriteSBitLong( int data, int numbits )
{
	// Do we have a valid # of bits to encode with?
	Assert( numbits >= 1 );

	// Note: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	if(data < 0)
	{
#ifdef _DEBUG
		if( numbits < 32 )
		{
			// Make sure it doesn't overflow.

			if( data < 0 )
			{
				Assert( data >= -(1 << (numbits-1)) );
			}
			else
			{
				Assert( data < (1 << (numbits-1)) );
			}
		}
#endif

		WriteUBitLong( (unsigned int)(0x80000000 + data), numbits - 1, false );
		WriteOneBit( 1 );
	}
	else
	{
		WriteUBitLong((unsigned int)data, numbits - 1);
		WriteOneBit( 0 );
	}
}

void bf_write::WriteBitLong(unsigned int data, int numbits, bool bSigned)
{
	if(bSigned)
		WriteSBitLong((int)data, numbits);
	else
		WriteUBitLong(data, numbits);
}

bool bf_write::WriteBits(const void *pInData, int nBits)
{


	unsigned char *pOut = (unsigned char*)pInData;
	int nBitsLeft = nBits;


	// Get output dword-aligned.
	while(((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		WriteUBitLong( *pOut, 8, false );
		++pOut;
		nBitsLeft -= 8;
	}

	// Read dwords.
	while(nBitsLeft >= 32)
	{
		WriteUBitLong( *((unsigned long*)pOut), 32, false );
		pOut += sizeof(unsigned long);
		nBitsLeft -= 32;
	}

	// Read the remaining bytes.
	while(nBitsLeft >= 8)
	{
		WriteUBitLong( *pOut, 8, false );
		++pOut;
		nBitsLeft -= 8;
	}

	// Read the remaining bits.
	if(nBitsLeft)
	{
		WriteUBitLong( *pOut, nBitsLeft, false );
	}

	return !IsOverflowed();
}


bool bf_write::WriteBitsFromBuffer( bf_read *pIn, int nBits )
{
	// This could be optimized a little by
	while ( nBits > 32 )
	{
		WriteUBitLong( pIn->ReadUBitLong( 32 ), 32 );
		nBits -= 32;
	}

	WriteUBitLong( pIn->ReadUBitLong( nBits ), nBits );
	return !IsOverflowed() && !pIn->IsOverflowed();
}




void bf_write::WriteChar(int val)
{
	WriteSBitLong(val, sizeof(char) << 3);
}

void bf_write::WriteByte(int val)
{
	WriteUBitLong(val, sizeof(unsigned char) << 3);
}

void bf_write::WriteShort(int val)
{
	WriteSBitLong(val, sizeof(short) << 3);
}

void bf_write::WriteWord(int val)
{
	WriteUBitLong(val, sizeof(unsigned short) << 3);
}

void bf_write::WriteLong(long val)
{
	WriteSBitLong(val, sizeof(long) << 3);
}

void bf_write::WriteFloat(float val)
{
	WriteBits(&val, sizeof(val) << 3);
}

bool bf_write::WriteBytes( const void *pBuf, int nBytes )
{
	return WriteBits(pBuf, nBytes << 3);
}

bool bf_write::WriteString(const char *pStr)
{
	if(pStr)
	{
		do
		{
			WriteChar( *pStr );
			++pStr;
		} while( *(pStr-1) != 0 );
	}
	else
	{
		WriteChar( 0 );
	}

	return !IsOverflowed();
}

// ---------------------------------------------------------------------------------------- //
// bf_read
// ---------------------------------------------------------------------------------------- //

bf_read::bf_read()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; // set to -1 so we overflow on any operation
	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

bf_read::bf_read( const void *pData, int nBytes, int nBits )
{
	m_bAssertOnOverflow = true;
	StartReading( pData, nBytes, 0, nBits );
}

bf_read::bf_read( const char *pDebugName, const void *pData, int nBytes, int nBits )
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartReading( pData, nBytes, 0, nBits );
}

void bf_read::StartReading( const void *pData, int nBytes, int iStartBit, int nBits )
{
	// Make sure we're dword aligned.
	Assert(((unsigned long)pData & 3) == 0);

	m_pData = (unsigned char*)pData;
	m_nDataBytes = nBytes;

	if ( nBits == -1 )
	{
		m_nDataBits = m_nDataBytes << 3;
	}
	else
	{
		Assert( nBits <= nBytes*8 );
		m_nDataBits = nBits;
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

void bf_read::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}

void bf_read::SetAssertOnOverflow( bool bAssert )
{
	m_bAssertOnOverflow = bAssert;
}

const char* bf_read::GetDebugName()
{
	return m_pDebugName;
}

void bf_read::SetDebugName( const char *pName )
{
	m_pDebugName = pName;
}

unsigned int bf_read::CheckReadUBitLong(int numbits)
{
	// Ok, just read bits out.
	int i, nBitValue;
	unsigned int r = 0;

	for(i=0; i < numbits; i++)
	{
		nBitValue = ReadOneBitNoCheck();
		r |= nBitValue << i;
	}
	m_iCurBit -= numbits;

	return r;
}

bool bf_read::ReadBits(void *pOutData, int nBits)
{


	unsigned char *pOut = (unsigned char*)pOutData;
	int nBitsLeft = nBits;


	// Get output dword-aligned.
	while(((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		*pOut = (unsigned char)ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// Read dwords.
	while(nBitsLeft >= 32)
	{
		*((unsigned long*)pOut) = ReadUBitLong(32);
		pOut += sizeof(unsigned long);
		nBitsLeft -= 32;
	}

	// Read the remaining bytes.
	while(nBitsLeft >= 8)
	{
		*pOut = ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// Read the remaining bits.
	if(nBitsLeft)
	{
		*pOut = ReadUBitLong(nBitsLeft);
	}

	return !IsOverflowed();
}

float bf_read::ReadBitAngle( int numbits )
{
	float fReturn;
	int i;
	float shift;

	shift = (float)( 1 << numbits );

	i = ReadUBitLong( numbits );
	fReturn = (float)i * (360.0 / shift);

	return fReturn;
}

unsigned int bf_read::PeekUBitLong( int numbits )
{
	unsigned int r;
	int i, nBitValue;
#ifdef BIT_VERBOSE
	int nShifts = numbits;
#endif

	bf_read savebf;

	savebf = *this;  // Save current state info

	r = 0;
	for(i=0; i < numbits; i++)
	{
		nBitValue = ReadOneBit();

		// Append to current stream
		if ( nBitValue )
		{
			r |= 1 << i;
		}
	}

	*this = savebf;

#ifdef BIT_VERBOSE
	Con_Printf( "PeekBitLong:  %i %i\n", nShifts, (unsigned int)r );
#endif

	return r;
}

// Append numbits least significant bits from data to the current bit stream
int bf_read::ReadSBitLong( int numbits )
{
	int r, sign;

	r = ReadUBitLong(numbits - 1);

	// Note: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	sign = ReadOneBit();
	if(sign)
		r = -((1 << (numbits-1)) - r);

	return r;
}


unsigned int bf_read::ReadBitLong(int numbits, bool bSigned)
{
	if(bSigned)
		return (unsigned int)ReadSBitLong(numbits);
	else
		return ReadUBitLong(numbits);
}



int bf_read::ReadChar()
{
	return ReadSBitLong(sizeof(char) << 3);
}

int bf_read::ReadByte()
{
	return ReadUBitLong(sizeof(unsigned char) << 3);
}

int bf_read::ReadShort()
{
	return ReadSBitLong(sizeof(short) << 3);
}

int bf_read::ReadWord()
{
	return ReadUBitLong(sizeof(unsigned short) << 3);
}

long bf_read::ReadLong()
{
	return ReadSBitLong(sizeof(long) << 3);
}

float bf_read::ReadFloat()
{
	float ret;
	Assert( sizeof(ret) == 4 );
	ReadBits(&ret, 32);
	return ret;
}

bool bf_read::ReadBytes(void *pOut, int nBytes)
{
	return ReadBits(pOut, nBytes << 3);
}

bool bf_read::ReadString( char *pStr, int maxLen, bool bLine, int *pOutNumChars )
{
	Assert( maxLen != 0 );

	bool bTooSmall = false;
	int iChar = 0;
	while(1)
	{
		char val = ReadChar();
		if ( val == 0 )
			break;
		else if ( bLine && val == '\n' )
			break;

		if ( iChar < (maxLen-1) )
		{
			pStr[iChar] = val;
			++iChar;
		}
		else
		{
			bTooSmall = true;
		}
	}

	// Make sure it's null-terminated.
	Assert( iChar < maxLen );
	pStr[iChar] = 0;

	if ( pOutNumChars )
		*pOutNumChars = iChar;

	return !IsOverflowed() && !bTooSmall;
}


char* bf_read::ReadAndAllocateString( bool *pOverflow )
{
	char str[2048];

	int nChars;
	bool bOverflow = !ReadString( str, sizeof( str ), false, &nChars );
	if ( pOverflow )
		*pOverflow = bOverflow;

	// Now copy into the output and return it;
	char *pRet = new char[ nChars + 1 ];
	for ( int i=0; i <= nChars; i++ )
		pRet[i] = str[i];

	return pRet;
}


bool bf_read::Seek(int iBit)
{
	if(iBit < 0)
	{
		SetOverflowFlag();
		m_iCurBit = m_nDataBits;
		return false;
	}
	else if(iBit > m_nDataBits)
	{
		SetOverflowFlag();
		m_iCurBit = m_nDataBits;
		return false;
	}
	else
	{
		m_iCurBit = iBit;
		return true;
	}
}

void bf_read::ExciseBits( int startbit, int bitstoremove )
{
	int endbit = startbit + bitstoremove;
	int remaining_to_end = m_nDataBits - endbit;

	bf_write temp;
	temp.StartWriting( (void *)m_pData, m_nDataBits << 3, startbit );

	Seek( endbit );

	for ( int i = 0; i < remaining_to_end; i++ )
	{
		temp.WriteOneBit( ReadOneBit() );
	}

	Seek( startbit );

	m_nDataBits -= bitstoremove;
	m_nDataBytes = m_nDataBits >> 3;
}
