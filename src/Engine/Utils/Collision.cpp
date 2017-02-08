#include "platform.h"
#include "Collision.h"
#include "Utils.h"
#include "Primitives.h"

const float CCollision::EPSILON = 0.0001f;

CArray<Vector3>*	CCollision::polygon;					
Vector3		CCollision::start;
Vector3		CCollision::size;
Vector3		CCollision::displacement;	
Vector3*	CCollision::dir;						
Vector3		CCollision::axis;						

bool		CCollision::collision;					
float		CCollision::mtdsquared;			
Vector3		CCollision::MTD;						
float		CCollision::tfirst;
Vector3		CCollision::Nfirst;
float		CCollision::tlast;
Vector3		CCollision::Nlast;

//////////////////////////////////////////////////////////////////////////
bool CCollision::LineBox( Vector3 p1, Vector3 p2, Vector3 min, Vector3 max)
{    
	Vector3 d = (p2 - p1) * 0.5f;    
	Vector3 e = (max - min) * 0.5f;    
	Vector3 c = p1 + d - (min + max) * 0.5f;
	Vector3 ad = Vector3( abs( d.x ), abs( d.y ), abs( d.z ) );

	if (p1 > min && p1 < max) return true;

	if ( abs(c.x) > e.x + ad.x) return false;    
	if (abs(c.y) > e.y + ad.y) return false;    
	if (abs(c.z) > e.z + ad.z) return false;      
	if (abs(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + EPSILON) return false; 
	if (abs(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + EPSILON) return false;   
	if (abs(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + EPSILON) return false; 
	return true;
}

//////////////////////////////////////////////////////////////////////////
float CCollision::DistanceToPlane( Vector3 v1, Vector3 v2, Vector3 v3, Vector3 point )
{
	Vector3 normal;
	Vector3 left = v2 - v1;
	Vector3 right = v3 - v1;
	normal = left.Cross(right);
	normal.Normalize();
	float dist = v1.Dot(normal);
	return point.Dot(normal) + dist;
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::FindRoots(float a, float b, float c, float& t0, float& t1)
{
	float d = b*b - (4.0f * a * c);

	if (d < 0.0f)
		return false;

	d = (float) sqrt(d);

	float one_over_two_a = 1.0f / (2.0f * a);

	t0 = (-b - d) * one_over_two_a;
	t1 = (-b + d) * one_over_two_a;

	if (t1 < t0)
	{
		float t = t0;
		t0 = t1;
		t1 = t;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::RayCircleIntersection(const Vector2& C, float r, const Vector2& O, const Vector2& D, float tmin, float tmax, float& t)
{
	float t0, t1;

	Vector2 H = (O - C);
	float  a = (D * D);
	float  b = (H * D) * 2.0f;
	float  c = (H * H) - r*r;

	if (!FindRoots(a, b, c, t0, t1))
		return false;

	if (t0 > tmax || t1 < tmin)
		return false;

	t = t0;

	if (t0 < tmin)
		t = t1;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::RaySphereIntersection(Vector3 origin, Vector3 &dir, Vector3 &center, float r, float &t)
{
	origin -= center;

	//Compute A, B and C coefficients
	float a = dir.Dot(dir);
	float b = 2 * dir.Dot(origin);
	float c = origin.Dot(origin) - (r * r);

	//Find discriminant
	float disc = b * b - 4 * a * c;

	// if discriminant is negative there are no real roots, so return 
	// false as ray misses sphere
	if (disc < 0)
		return false;

	// compute q as described above
	float distSqrt = sqrtf(disc);
	float q;
	if (b < 0)
		q = (-b - distSqrt)/2.0;
	else
		q = (-b + distSqrt)/2.0;

	// compute t0 and t1
	float t0 = q / a;
	float t1 = c / q;

	// make sure t0 is smaller than t1
	if (t0 > t1)
	{
		// if t0 is bigger than t1 swap them around
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	// if t1 is less than zero, the object is in the ray's negative direction
	// and consequently the ray misses the sphere
	if (t1 < 0)
		return false;

	// if t0 is less than zero, the intersection point is at t1
	if (t0 < 0)
	{
		t = t1;
		return true;
	}
	// else the intersection point is at t0
	else
	{
		t = t0;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::LineTriangle(Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector3 &start, Vector3 &end, float *fraction)
{
	Vector3 u = v2-v1;
	Vector3 v = v3-v1;
	Vector3 n = Vector3::Cross(u,v);

	if (n.LengthSq() == 0) return false;

	Vector3 dir = end - start;
	Vector3 w0 = start - v1;
	float a = -Vector3::Dot(n, w0);
	float b =  Vector3::Dot(n, dir);

	if (fabs(b) < 0.0001f) return false;
	
	float r = a / b;
	if ( r < 0) return false;
	if (r > 1) return false;

	Vector3 I = start + r * dir;
	float uu, uv, vv, wu, wv, D;
	uu = u.Dot(u);
	uv = u.Dot(v);
	vv = v.Dot(v);
	Vector3 w = I - v1;
	wu = w.Dot(u);
	wv = w.Dot(v);
	D = uv * uv - uu * vv;

	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)        // I is outside T
		return false;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return false;

	if (fraction) *fraction = r;
	return true;                      // I is in T
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::LineTriangles(CArray<STriangle> &tris, Vector3 &start, Vector3 &end, int &index)
{
	for (int i=0; i<tris.Size(); i++)
	{
		if (LineTriangle(tris[i].v[0], tris[i].v[1], tris[i].v[2], start, end))
		{
			index = i;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CCollision::OBBPolygon( CArray<Vector3> &polygon, Vector3 start, Vector3 size, Vector3 dir[3], Vector3 displacement, OUT SCollisionResult &result )
{
	// copy to internal state
	CCollision::polygon = &polygon;
	CCollision::start = start;
	CCollision::size = size;
	CCollision::dir = dir;
	CCollision::displacement = displacement;


	// reset the variables
	const float tolerance = 0.001f; //1.0E-8f;
	collision = false;
	mtdsquared = -1.0f;
	MTD = Vector3();
	tfirst = 0.0f - tolerance;
	Nfirst = Vector3();
	tlast = 1.0f + tolerance;
	Nlast = Vector3();

	// get polygon normal and test using this axis

	Vector3 cross = axis = (polygon[2] - polygon[1]).Cross(polygon[1] - polygon[0]);
	if ( !IntervalCollision() ) return false;

	// test all 3 axis of the box
	for ( int i=0; i < 3; i++ )
	{
		axis = dir[i];
		if ( !IntervalCollision() ) return false;
	}

	if ( collision )
	{
		// check if we found a valid time of collision
		if ( tfirst < 0.0f || tfirst > 1.0f )
			return false;

		MTD = Vector3();
		Nfirst.Normalize();
	}
	else
	{
		tfirst = 0.0f;
		Nfirst = MTD;
		Nfirst.Normalize();
	}

	result.collision = collision;
	result.fraction = tfirst;
	result.MTD = MTD;
	result.normal = Nfirst;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Check if the polygon and box overlap when projected onto the current axis
bool CCollision::IntervalCollision()
{
	float amin=0, amax=0;		// polygon projected range
	float bmin=0, bmax=0;		// box projected range
	CalculatePolygonIntervalOnAxis( amin, amax );
	CalculateBoxIntervalOnAxis( bmin, bmax );

	float disp  = Vector3::Dot( displacement, axis );		// how far do we move along this particular axis

	return SlabSlabCollision( amin, amax, bmin, bmax, disp );
}

//////////////////////////////////////////////////////////////////////////
// Check if two projected ranges overlap and calculate how much if they do
bool CCollision::SlabSlabCollision( float amin, float amax, float bmin, float bmax, float displacement )
{
	float dist_0 = amin - bmax;			// how far we need to push to the left
	float dist_1 = amax - bmin;			// how far we need to push to the right

	bool bIntersect = (dist_0 < 0.0f && dist_1 > 0.0f);		// do the ranges intersect?

	// we are already intersecting, we need to be pushed outside ASAP
	if ( bIntersect )
	{
		// the vector we use to push the point refside the slab
		float axisMtd			= (abs( dist_0 ) < abs( dist_1 )) ? dist_0 : dist_1;		// distance we are gonna push the box back along this axis
		float axisLengthSquared = Vector3::Dot( axis, axis );										// square length of this axis
		Vector3 axisMtd3D		= axis * (axisMtd / axisLengthSquared);								// 3d push vector
		float axisMtd3DLengthSquared	= Vector3::Dot( axisMtd3D, axisMtd3D );

		// Check if that push vector is smaller than our curent push vector
		if ( axisMtd3DLengthSquared < mtdsquared || mtdsquared < 0.0f )
		{
			MTD = axisMtd3D;
			mtdsquared = axisMtd3DLengthSquared;
		}
	}

	// if not moving, then we can only collide through intersections
	if ( abs( displacement ) < 1.0E-8f )
		return bIntersect;

	// We are not collising yet, check if we colide when we move
	float tslab0 = (dist_0) / (displacement);		// how many steps are needed to push out way out to the left
	float tslab1 = (dist_1) / (displacement);		// and to the right
	float tslabenter;
	float tslabexit;
	float sign;

	// we are entering collision from the left
	if ( tslab0 < tslab1 )
	{
		tslabenter = tslab0;
		tslabexit = tslab1;
		sign = -1.0f;
	}
	else
	{
		// or from the right
		tslabenter = tslab1;
		tslabexit = tslab0;
		sign = 1.0f;
	}

	// did this collision happen sooner then any other collision?
	if ( tslabenter > tfirst )
	{
		tfirst = tslabenter;
		Nfirst = axis * sign;
		collision = true;
	}

	if ( tslabexit < tlast )
	{
		tlast = tslabexit;
		Nlast = axis * -sign;
	}

	if ( tlast < tfirst )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Project polygon onto the axis and get the range
void CCollision::CalculatePolygonIntervalOnAxis( float &min, float &max )
{
	min = 0; max = 0;
	min = max = Vector3::Dot( (*polygon)[0], axis );

	for ( int i = 1; i < polygon->Size(); i++ )
	{
		float p = Vector3::Dot( (*polygon)[i], axis );
		if ( p < min ) min = p;
		else if ( p > max ) max = p;
	}
}

//////////////////////////////////////////////////////////////////////////
// Project box onto the axis and get the range
void CCollision::CalculateBoxIntervalOnAxis( float &min, float &max )
{
	float p = Vector3::Dot( start, axis );
	float rx = abs( Vector3::Dot( dir[0], axis ) ) * size.x;
	float ry = abs( Vector3::Dot( dir[1], axis ) ) * size.y;
	float rz = abs( Vector3::Dot( dir[2], axis ) ) * size.z;
	float r  = rx + ry + rz;
	min = p - r;
	max = p + r;
}


