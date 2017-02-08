#pragma once
#include "platform.h"
#include "Utils.h"
#include "Array.h"

struct SCollisionResult;
struct STriangle;
class Vector3;

//////////////////////////////////////////////////////////////////////////
class CMovement
{
public:
	static Vector3			SlideMove( CArray<STriangle> &triangles, Vector3 start, Vector3 bboxMax, Vector3 boxdir[3], Vector3 displacement, OUT SCollisionResult &result );
	static SCollisionResult	Trace( CArray<STriangle> &triangles, Vector3 start, Vector3 bboxMax, Vector3 boxdir[3], Vector3 displacement );

private:
	static Vector3			ClipVelocity( Vector3 dir, Vector3 normal, float overbounce );
};