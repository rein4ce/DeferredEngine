#include "platform.h"
#include "Physics.h"
#include "Object3D.h"
#include <Newton.h>
#include "VGUI.h"
#include "Renderer.h"
#include "Scene.h"
#include "Object3D.h"
#include "Material.h"
#include "Mesh.h"
#include "Geometries.h"
#include "Vector.h"

//////////////////////////////////////////////////////////////////////////
NewtonBody* CPhysics::CreateBox(NewtonWorld *world, CObject3D *object, float x, float y, float z, float mass, Vector3 offset)
{
	Matrix4 offsetMat;
	offsetMat.SetPosition(offset);
	float m[16];
	offsetMat.FlattenToArray(m);

	NewtonCollision *collision = NewtonCreateBox(world, x, y, z, 0, m);
	return CreateRigidBody(world, object, collision, mass);	
}

//////////////////////////////////////////////////////////////////////////
NewtonBody*	CPhysics::CreateRigidBody(NewtonWorld *world,  CObject3D *object, NewtonCollision *collision, float mass)
{
	object->UpdateMatrix();
	float matrix[16];
	object->matrixModel.FlattenToArray(matrix);
	Vector3 angles = object->rotation * ToRad;

	NewtonBody *body = NewtonCreateBody(world, collision, matrix);
	

	if (mass > 0)
	{
		float inertia[3], origin[3];
		NewtonConvexCollisionCalculateInertialMatrix (collision, &inertia[0], &origin[0]);	
		NewtonBodySetMassMatrix(body, mass, mass * inertia[0], mass * inertia[1], mass * inertia[2]);
		NewtonBodySetCentreOfMass(body, &origin[0]);
	}	

	NewtonBodySetUserData(body, object);
	NewtonBodySetTransformCallback(body, CPhysics::TransformCallback);
	NewtonBodySetDestructorCallback(body, CPhysics::DestroyBodyCallback);

	NewtonReleaseCollision(world, collision);
	
	return body;
}

//////////////////////////////////////////////////////////////////////////
void CPhysics::TransformCallback(const NewtonBody* body, const float* matrix, int threadIndex)
{
	CObject3D *object = (CObject3D*)NewtonBodyGetUserData(body);
	float m[16];
	NewtonBodyGetMatrix(body, m);	

	
	Matrix4 M = Matrix4(m);
	Vector3 destination = M.GetPosition();
	Vector3 velocity = (destination - object->GetWorldPosition()) * object->physicsDelay;
	object->velocity = velocity;

	Vector3 pos = object->position;
	object->SetMatrix( Matrix4(m) );
	object->SetPosition(pos);

	// setting entire matrix each physics frame might cause desync with the renderer,
	// that might run more often, resulting in drawing the object in the same location
	// twice in a row

	// Instead each physics frame we set the object's velocity so that it reaches
	// the desginated position in 1/10th of a second	
}

//////////////////////////////////////////////////////////////////////////
void CPhysics::DestroyBodyCallback( const NewtonBody* body )
{
	//trace("DESTROY !!!");
}

//////////////////////////////////////////////////////////////////////////
NewtonBody* CPhysics::CreateSphere( NewtonWorld *world, CObject3D *object, float radius, float mass /*= 0.0f*/ )
{
	NewtonCollision *collision = NewtonCreateSphere(world, radius, radius, radius, 0, NULL);
	return CreateRigidBody(world, object, collision, mass);	
}

//////////////////////////////////////////////////////////////////////////
void BoxGravityCallback (const NewtonBody* body, float timestep, int threadIndex)
{
	float mass, ix, iy, iz;

	NewtonBodyGetMassMatrix(body, &mass, &ix, &iy, &iz);
	Vector4 gravityForce = Vector4(0.0f, mass * GRAVITY, 0.0f, 1.0f);
	NewtonBodySetForce(body, (float*)&gravityForce);
}

//////////////////////////////////////////////////////////////////////////
NewtonBody* AddBox(CScene *pScene, NewtonWorld *pWorld, Vector3 pos, Vector3 size, Vector3 rot, float mass)
{
	static map<string, CGeometry*> geometries;
	static CMaterial *material = NULL;

	if (!material)
	{
		material = new CMaterial("textures/box2.png");
		//material->features = EShaderFeature::LIGHT | EShaderFeature::FOG | EShaderFeature::SHADOW;	
	}

	string name = str("%f_%f_%f", size.x, size.y, size.z);
	if (geometries.find(name) == geometries.end())
	{
		CGeometry *g = new CCubeGeometry(size.x, size.y, size.z);
		g->materials.AddToTail(material);
		geometries[name] = g;		
	}

	CMesh *box = new CMesh( geometries[name] );

	box->SetPosition(pos);
	box->SetRotation(rot);	
	NewtonBody *body = CPhysics::CreateBox(pWorld, box, size.x, size.y, size.z, mass);

	pScene->Add(box);
	NewtonBodySetForceAndTorqueCallback(body, BoxGravityCallback);
	NewtonBodySetAutoSleep(body, 1);			// make boxes go to sleep automatically (default)
	return body;
}

//////////////////////////////////////////////////////////////////////////
NewtonBody* AddSphere(CScene *pScene, NewtonWorld *pWorld, Vector3 pos, float radius, float mass)
{
	static map<float, CGeometry*> geometries;
	static CMaterial *material = NULL;

	if (!material)
	{
		material = new CMaterial();
	}

	if (geometries.find(radius) == geometries.end())
	{
		CGeometry *g = new CSphereGeometry(radius);
		g->materials.AddToTail(material);
		geometries[radius] = g;
	}

	CMesh *sphere = new CMesh( geometries[radius] );
	sphere->SetPosition(pos);
	NewtonBody *body = CPhysics::CreateSphere(pWorld, sphere, radius, mass);
	pScene->Add(sphere);
	NewtonBodySetForceAndTorqueCallback(body, BoxGravityCallback);
	return body;
}


// Utility function to calculate the Bounding Box of a collision shape 
void CalculateNewtonBoundingBox (const NewtonCollision* shape, Vector3& boxMin, Vector3& boxMax)
{
	// for exact axis find minimum and maximum limits
	for (int i = 0; i < 3; i ++) {
		Vector3 point;
		Vector3 dir (0.0f, 0.0f, 0.0f);

		// find the most extreme point along this axis and this is maximum Box size on that direction
		dir[i] = 1.0f;
		NewtonCollisionSupportVertex (shape, &dir[0], &point[0]);
		boxMax[i] = point.Dot(dir);

		// find the most extreme point along the opposite axis and this is maximum Box size on that direction
		dir[i] = -1.0f;
		NewtonCollisionSupportVertex (shape, &dir[0], &point[0]);
		boxMin[i] = -(point.Dot(dir));
	}
}