#pragma once
#include "platform.h"
#include "Utils.h"
#include "Vector.h"
#include "Array.h"

struct STriangle;
struct SPlane;

//////////////////////////////////////////////////////////////////////////
struct SCollisionResult
{
	bool			collision;				// was there a collision?
	Vector3			MTD;					// pushing vector
	Vector3			normal;					// normal of the hitting surface
	float			fraction;				// fraction of the distance travelled
	Vector3			endpoint;				// final position
	Vector3			slide;
};

//////////////////////////////////////////////////////////////////////////
class CCollision
{
public:
	static const float EPSILON;
	
	static bool		LineBox(Vector3 p1, Vector3 p2, Vector3 min, Vector3 max);
	static float	DistanceToPlane( Vector3 v1, Vector3 v2, Vector3 v3, Vector3 point );
	static bool		RayCircleIntersection(const Vector2& C, float r, const Vector2& O, const Vector2& D, float tmin, float tmax, float& t);	
	static bool		LineTriangle(Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector3 &start, Vector3 &end, float *fraction = NULL);
	static bool		LineTriangles(CArray<STriangle> &tris, Vector3 &start, Vector3 &end, int &index);
	static bool		OBBPolygon( CArray<Vector3> &polygon, Vector3 start, Vector3 size, Vector3 dir[3], Vector3 displacement, OUT SCollisionResult &result );
	static bool		RaySphereIntersection(Vector3 origin, Vector3 &dir, Vector3 &center, float r, float &t);
	
private:
	static bool		FindRoots(float a, float b, float c, float& t0, float& t1);
	static bool		IntervalCollision();
	static bool		SlabSlabCollision( float amin, float amax, float bmin, float bmax, float displacement );
	static void		CalculatePolygonIntervalOnAxis( float &min, float &max );
	static void		CalculateBoxIntervalOnAxis( float &min, float &max );

private:
	static CArray<Vector3>		*polygon;					
	static Vector3		start, size, displacement;	
	static Vector3*		dir;						
	static Vector3		axis;						

	static bool			collision;					
	static float		mtdsquared;			
	static Vector3		MTD;						
	static float		tfirst;
	static Vector3		Nfirst;
	static float		tlast;
	static Vector3		Nlast;
};