#include "platform.h"
#include "Controllers.h"
#include "Collision.h"
#include "Primitives.h"
#include "Renderer.h"
#include "CVar.h"
#include "Input.h"
#include "Game.h"

CVar cv_mouseSpeed = CVar("mouseSpeed", "0.01", FCVAR_ARCHIVE, "Mouse rotation speed multiplier");

//////////////////////////////////////////////////////////////////////////
CController::CController(CObject3D *target /* = NULL */)
{
	pTarget = target;
	maxSpeed = 10;
	falloff = 0.5f;
	threshold = 0.01f;
}

//////////////////////////////////////////////////////////////////////////
// Update position of the camera given the destination point
//////////////////////////////////////////////////////////////////////////
void CController::Update(float frametime )
{	
	pTarget->SetPosition( Lerp(pTarget->position, destPosition, step) );
	pTarget->SetDirection( Slerp(pTarget->forward, destForward, step) );
}

//////////////////////////////////////////////////////////////////////////
void CFlyController::Update(float frametime )
{
	if (!pTarget || frametime == 0.0f) return;

	maxSpeed = (gInput.IsKeydown(K_SHIFT) ? 50.0f : 10.0f);

	// Check input
	Vector3 v = pTarget->velocity;
	if (gInput.IsKeydown('w') ) v += pTarget->forward * maxSpeed;
	if (gInput.IsKeydown('s') ) v -= pTarget->forward * maxSpeed;
	if (gInput.IsKeydown('a') ) v -= pTarget->right * maxSpeed;
	if (gInput.IsKeydown('d') ) v += pTarget->right * maxSpeed;
	if (gInput.IsKeydown(K_SPACE) ) v += pTarget->up * maxSpeed;
	if (gInput.IsKeydown(K_CTRL) ) v -= pTarget->up * maxSpeed;

	// cap the speed to the limit
	if (v.Length() > maxSpeed)
	{ 
		float scale = maxSpeed / v.Length();
		v *= scale;
	}

	pTarget->velocity = v;

	// Apply movement
	pTarget->Update(frametime);

	// Fall off
	Vector3 o = pTarget->velocity;
	v = pTarget->velocity * falloff * 20.0f * frametime;
	pTarget->velocity -= v;

	// if speed is too low, just stop
	if (pTarget->velocity.LengthSq() < 0.01f) pTarget->velocity.Set(0,0,0);

	// Mouse rotation
	if (!gInput.m_bCenterCursor) return;

	//gInput.SetCenterCursor(true);
	float ms = cv_mouseSpeed.GetFloat();
	float dx = gInput.GetDeltaX() * ms;
	float dy = gInput.GetDeltaY() * ms;
	pTarget->Rotate( dy, dx, 0 );

	float angleMargin = 15.0f;

	// limit angles
	if (pTarget->rotation.x < 270.0f+angleMargin && pTarget->rotation.x > 180.0f) 
		pTarget->SetRotationX( 270.0f+angleMargin );
	if (pTarget->rotation.x > 90.0f-angleMargin && pTarget->rotation.x < 180.0f)
		pTarget->SetRotationX( 90.0f-angleMargin );

	pTarget->updateMatrix = true;
}

CVar cv_zoomspeed = CVar("zoomspeed", "20", FCVAR_ARCHIVE);

//////////////////////////////////////////////////////////////////////////
COrbitController::COrbitController( CObject3D *camera, CObject3D *lookAt, Vector3 angle, float distance, Vector3 offset ) : CController(camera)
{
	this->lookAt = lookAt;
	this->targetDistance = this->distance = distance;
	this->targetAngle = this->angle = angle;	
	this->offset = offset;

	this->rotationSpeed = 0.25f;
	this->distanceSpeed = cv_zoomspeed.GetFloat();
	
	Vector3 pos = lookAt->GetPosition();

	minDistance = 2.0f;
	maxDistance = 50.0f;

	pTarget->SetPosition(pos);
	pTarget->SetRotation(lookAt->GetRotation());
	pTarget->MoveForward(offset.z);
	pTarget->MoveRight(offset.x);
	pTarget->MoveUp(offset.y);
	pTarget->Rotate(angle.x, angle.y, angle.z);
	pTarget->MoveForward(-distance);
}


//////////////////////////////////////////////////////////////////////////
void COrbitController::Update(float frametime )
{
	Vector3 angleDisplacement;

	if (!gInput.m_bCenterCursor) return;

	float dx = gInput.GetDeltaX();
	float dy = gInput.GetDeltaY();
	dx *= 0.5f;
	dy *= 0.5f;

	// how much angle has changed
	angleDisplacement.y += dx;
	angleDisplacement.x += dy;
	targetAngle += angleDisplacement;

	// verify angle
	Clamp(targetAngle.x, -91.0f, +91.0f);
	
	angle.y = Lerp(angle.y, targetAngle.y, rotationSpeed);
	angle.x = Lerp(angle.x, targetAngle.x, rotationSpeed);
	
	// adjust distance
	if (KeyPressed(K_MWHEELDOWN)) targetDistance += 1.0f;
	if (KeyPressed(K_MWHEELUP)) targetDistance -= 1.0f;
	Clamp(targetDistance, minDistance, maxDistance);

	// adjuts the actual angle towards the goal	
	distance = Lerp(distance, targetDistance, cv_zoomspeed.GetFloat() * frametime);

	// point we are orbiting around
	Vector3 center = lookAt->GetWorldPosition() + offset;

	pTarget->SetPosition(center);
	pTarget->SetRotation(angle);
	pTarget->MoveForward(-distance);

	// check current position against terrain
	Vector3 currentPos = pTarget->position;	

	SRayCastResult result;
	bool hit = pGame->CastRay(center, currentPos, RC_TERRAIN | RC_VOXEL, OUT result);
	if (hit)
	{		
		pTarget->SetPosition(result.hitPos);
		pTarget->MoveForward(4.0f);
	}
}

