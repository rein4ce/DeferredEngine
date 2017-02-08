#pragma once
#include "Vector.h"

class CObject3D;

class CController 
{
public:
	CController( CObject3D *target = NULL );

	virtual void		Update(float frametime);

	inline CObject3D*	GetTarget() { return pTarget; }
	inline void			SetTarget( CObject3D *object ) { pTarget = object; }
	inline void			SetMaxSpeed( float speed ) { maxSpeed = speed; }

public:
	float			step;					// fraction of the movement each frame
	CObject3D		*pTarget;				// the object we will move
	float			maxSpeed;
	float			falloff;				// how much we slow down
	float			threshold;				// what's the minimum speed below which we stop
	Vector3			destPosition;			// destination where we want to move
	Vector3			destForward;			// desired forward vector
};

//////////////////////////////////////////////////////////////////////////
class CFlyController : public CController
{
public:
	CFlyController( CObject3D *target ): CController(target) {}

	virtual void Update(float frametime );
};

//////////////////////////////////////////////////////////////////////////
class COrbitController : public CController
{
public:
	COrbitController(CObject3D *camera, CObject3D *lookAt, Vector3 angle, float distance, Vector3 offset = Vector3());
	virtual void		Update(float frametime );

	CObject3D	*lookAt;				// look at object
	float		distance;				// current distance
	Vector3		angle;					// where we are pointing now
	Vector3		angleVel;	
	
	Vector3		offset;

	float		rotationSpeed;			// rotation speed multipler
	float		distanceSpeed;			// how fast we are changing distance

	Vector3		targetAngle;			// where we want to point in the end
	float		targetDistance;
	float		minDistance, maxDistance;
};

