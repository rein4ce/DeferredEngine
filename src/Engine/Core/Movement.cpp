#include "platform.h"
#include "Movement.h"
#include "Utils.h"
#include "Collision.h"
#include "Face.h"
#include "Plane.h"
#include "Primitives.h"
#include "Array.h"
#include "Vector.h"

//////////////////////////////////////////////////////////////////////////
Vector3 CMovement::SlideMove( CArray<STriangle> &triangles, Vector3 start, Vector3 bboxMax, Vector3 boxdir[3], Vector3 displacement, OUT SCollisionResult &trace )
{
	// try to move in the desired direction
	trace = Trace( triangles, start, bboxMax, boxdir, displacement );

	if ( !trace.collision ) return trace.endpoint;

	// we colliding and started moving along the next surface
	for ( int i=0; i < 3; i++ )
	{
		start = trace.endpoint;
		trace = Trace( triangles, start, bboxMax, boxdir, trace.slide );

		if ( !trace.collision ) return trace.endpoint;
	}

	return trace.endpoint;
}

//////////////////////////////////////////////////////////////////////////
SCollisionResult CMovement::Trace( CArray<STriangle> &triangles, Vector3 start, Vector3 bboxMax, Vector3 boxdir[3], Vector3 displacement )
{
	float fraction = 1.0f;
	SCollisionResult trace, closest;
	float dist, mindist = 99999.0f;
	CArray<STriangle> colliding;

	ZeroMemory(&closest, sizeof(closest));
	ZeroMemory(&trace, sizeof(trace));

	// Find the collisions with the closest triangles	
	for (int i=0; i<triangles.Size(); i++)
	{
		STriangle &t = triangles[i];
		
		CArray<Vector3> vlist;
		vlist.AddToTail(t.v[0]);
		vlist.AddToTail(t.v[1]);
		vlist.AddToTail(t.v[2]);

		if ( CCollision::OBBPolygon( vlist, start, bboxMax, boxdir, displacement, OUT trace ) )
		{
			if ( trace.fraction <= fraction + 0.1f )
			{
				// check if all points of our bbox are in front of the face
				SPlane plane = SPlane( t.v[0], t.v[1], t.v[2] );
				Vector3 size = bboxMax * 2.0f;
				Vector3 min = start - bboxMax;
				Vector3 max = start + bboxMax;

				if ( plane.GetSide( min, OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( max, OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( min + Vector3( size.x, 0, 0 ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( min + Vector3( 0, size.y, 0 ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( min + Vector3( 0, 0, size.z ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( max - Vector3( size.x, 0, 0 ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( max - Vector3( 0, size.y, 0 ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				if ( plane.GetSide( max - Vector3( 0, 0, size.z ), OUT dist ) == EPlaneSide::Back ) continue; if ( dist < mindist ) mindist = dist;
				
				fraction = trace.fraction;
				closest = trace;
				t.normal = -(t.v[2] - t.v[0]).Cross(t.v[1] - t.v[0]).Normalize();
				closest.normal = t.normal;
				colliding.AddToTail( t );				// add triangle to colliding triangles list	
			}
		}
	}

	Vector3 dir = displacement;
	Vector3 normal = closest.normal;
	Vector3 endpoint = start;

	// If collided, process the collision
	if ( fraction < 1 )
	{
		// OK
		float m = 0.005f;								// distance from the surface
		float f = closest.fraction;						// fraction of the full dir distance to the contact point
		Vector3 dirNorm = dir; dirNorm.Normalize();
		if ( Vector3::Dot( -dir, normal ) != 0 )
		{
			float ndir = dir.Length() * (m / Vector3::Dot( -dir, normal ));
			float x = (f * dir.Length() - ndir);				// distance we move along the dir direction to stay away from the surface

			endpoint = start + dirNorm * x;

			Vector3 slide = ClipVelocity( dir, closest.normal, 1.00f );
			float left = 1.0f - (f - ndir / dir.Length());

			closest.slide = slide * left;
		}
	}
	else
		endpoint = start + dir;

	closest.collision = fraction != 1.0f;
	closest.endpoint = endpoint;
	return closest;
}

//////////////////////////////////////////////////////////////////////////
Vector3 CMovement::ClipVelocity( Vector3 dir, Vector3 normal, float overbounce )
{			
	float backoff = Vector3::Dot(dir, normal);
	if ( backoff < 0 )
		backoff *= overbounce;
	else
		backoff /= overbounce;

	return Vector3(
		dir.x - normal.x * backoff,
		dir.y - normal.y * backoff,
		dir.z - normal.z * backoff
		);
}