#pragma once
#include "platform.h"
#include "Vector.h"

namespace EPlaneSide { enum Type
{
	Front,
	Back,
	OnPlane
}; };


struct SPlane
{
	Vector3		normal;
	float		dist;

	SPlane(Vector3 a, Vector3 b, Vector3 c)
	{
		Vector3 left = c-a;
		Vector3 right = b-a;
		normal = -Vector3::Cross(left, right).Normalize();
		dist = -Vector3::Dot(a, normal);
	}

	float GetDistance(Vector3 point)
	{
		return Vector3::Dot(point, normal) + dist;
	}

	EPlaneSide::Type GetSide( Vector3 pt )
	{
		float dist = GetDistance( pt );
		
		if ( dist > 0.01f )
			return EPlaneSide::Front;
		else if ( dist < -0.01f )
		{ return EPlaneSide::Back; }
		else
			return EPlaneSide::OnPlane;
	}

	EPlaneSide::Type GetSide( Vector3 pt, OUT float &d )
	{
		d = GetDistance( pt );
		
		if ( d > 0.001f )
			return EPlaneSide::Front;
		else if ( d < -0.001f )
		{ return EPlaneSide::Back; }
		else
			return EPlaneSide::OnPlane;
	}
};