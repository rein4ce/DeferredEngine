#pragma once

#include "platform.h"
#include <D3D10.h>

inline const char* VectorToString( D3DXVECTOR3 vec )
{
	char s[128];
	sprintf(s, "%4.3f, %4.3f, %4.3f", vec.x, vec.y, vec.z);
	return s;
}