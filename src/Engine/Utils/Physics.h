#pragma once
#include "platform.h"
#include "Utils.h"
#include "Vector.h"


#define GRAVITY -10.0f

class NewtonWorld;
class NewtonBody;
class NewtonCollision;
class CObject3D;

class CPhysics
{
public:

	static NewtonBody*		CreateSphere(NewtonWorld *world, CObject3D *object, float radius, float mass = 0.0f);
	static NewtonBody*		CreateBox(NewtonWorld *world, CObject3D *object, float x, float y, float z, float mass = 0.0f, Vector3 offset = Vector3());
	static NewtonBody*		CreateRigidBody(NewtonWorld *world, CObject3D *object, NewtonCollision *collision, float mass = 0.0f);

	static void				DestroyBodyCallback(const NewtonBody* body);
	static void				TransformCallback(const NewtonBody* body, const float* matrix, int threadIndex);
};

void BoxGravityCallback (const NewtonBody* body, float timestep, int threadIndex);
void CalculateNewtonBoundingBox (const NewtonCollision* shape, Vector3& boxMin, Vector3& boxMax);