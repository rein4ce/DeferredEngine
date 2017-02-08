#pragma once
#include "platform.h"
#include "Vector.h"
#include <D3DX10math.h>

//////////////////////////////////////////////////////////////////////////
// 3D Utils
//////////////////////////////////////////////////////////////////////////
namespace EBlending
{
	enum Type
	{
		Normal = 0,
		Additive,
		AdditiveLights,
		Subtractive,
		None
	};
}

namespace EShading
{
	enum Type
	{
		NoShading = 0,
		Flat,
		Smooth
	};
}

namespace ETextureOperation
{
	enum Type
	{
		Multiply = 0,
		Mix
	};
}


struct SBBox 
{
	Vector3 min, max;

	SBBox() {}
	SBBox( Vector3 min, Vector3 max ) 
	{
		this->min = min;
		this->max = max;
	};
	inline Vector3 GetSize() { return max - min; }
	inline Vector3 GetCenter() { return min + (max-min)/2.0f; }
};

