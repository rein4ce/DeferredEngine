#include "platform.h"
#include "Car.h"
#include "ModelLoader.h"
#include "Object3D.h"
#include "Utils.h"
#include "Mesh.h"
#include "Input.h"
#include "Geometries.h"
#include "Material.h"
#include "Physics.h"
#include "Game.h"
#include <Newton.h>
#include "VGUI.h"
#include "ParticleSystem.h"
#include "Input.h"
#include "Game.h"
#include "Controllers.h"
#include "Renderer.h"
#include "Terrain.h"
#include "Scene.h"
#include "CVar.h"
#include "Vector.h"
#include "Array.h"
#include "JointLibrary.h"
#include "Newton.h"
#include "..\NewtonWin-2.30\sdk\dMath\dMatrix.h"
#include "..\NewtonWin-2.30\sdk\dMath\dVector.h"


CVar cv_torque("torque", "1.0", FCVAR_ARCHIVE);
CVar cv_boost("boost", "5000.0", FCVAR_ARCHIVE);
CVar cv_angle("angle", "15.0", FCVAR_ARCHIVE);
CVar cv_maxTorque("maxTorque", "20000", FCVAR_ARCHIVE);
CVar cv_torqueStart("torqueStart", "3", FCVAR_ARCHIVE);
CVar cv_speedDivider("speeddiv", "1.0", FCVAR_ARCHIVE);
CVar cv_drag("drag", "1.0", FCVAR_ARCHIVE);
CVar cv_brake("brake", "10.0", FCVAR_ARCHIVE);
CVar cv_angleDampening("angleDampening", "4.0", FCVAR_ARCHIVE);		// how much speed affects max wheelAngle


void GravityCallback (const NewtonBody* body, float timestep, int threadIndex)
{
	float mass, ix, iy, iz;

	NewtonBodyGetMassMatrix(body, &mass, &ix, &iy, &iz);
	Vector4 gravityForce = Vector4(0.0f, mass * GRAVITY*10, 0.0f, 1.0f);
	
	CCarEntity* car = (CCarEntity*)NewtonBodyGetUserData(body);

	NewtonBodySetVelocity(body, &car->intendedMove[0]);
}

/////////////////////////////////////////////////////////////////////////
void CarTransformCallback(const NewtonBody* body, const float* matrix, int threadIndex)
{
	CCarEntity *object = (CCarEntity*)NewtonBodyGetUserData(body);
	float m[16];
	NewtonBodyGetMatrix(body, m);	

	EnterCriticalSection(&renderCS);
	object->SetMatrix( Matrix4(m) );
	object->SetRotation(object->intendedRotation);
	pGame->pCameraController->Update(pGame->frametime);
	LeaveCriticalSection(&renderCS);			

}

CCarEntity::CCarEntity()
{
	// Car parameters	
	Vector3 carSize = Vector3(1.0f, 0.5f, 2.0f);
	float carWheelRadius = 0.35f;
	float carWheelThickness = 0.3f;
	Vector3 carWheelOffset = Vector3(0.18f, -0.55f, 0.15f);

	// Load a car model and position it inside the container
	pCarMesh = CModelLoader::Get("models/asv/asv.obj");
	CObject3D *carContainer = new CObject3D();
	carContainer->Add(pCarMesh);
	carContainer->SetRotationX(90);
	carContainer->SetRotationZ(-180);
	carContainer->SetScale(0.1f);
	pCarMesh->MoveUp(1.75f);			// move forward
	pCarMesh->MoveForward(-2);			// move down
	pCarMesh->frustumCulled = false;
	pCarMesh->boundingShape = EBoundingShape::Sphere;
	pCarMesh->CalculateBoundingShape();


	this->Add(carContainer);
	this->SetPosition(-5,2,-5);			// Elevate the car a little bit	

	// Minigun
	CObject3D *barrelMesh = CModelLoader::Get("models/minigun.3ds");
	barrelMesh->SetScale(0.0004f);
	barrelMesh->SetRotationX(90);
	barrelMesh->SetRotationZ(180);	
	pBarrel = new CObject3D();
	pBarrel->SetPosition(0.35f,0.45f,0.3f);	
	pBarrel->Add(barrelMesh);

	this->Add(pBarrel);

	// Find turret
	pTurret = pCarMesh->GetChildByName("2");
	((CMesh*)pTurret)->Reposition();

	// Car wheels
	pCarWheels[0] = pCarMesh->GetChildByName(str("%d", 19)); // front left
	pCarWheels[1] = pCarMesh->GetChildByName(str("%d", 18)); // front right
	pCarWheels[2] = pCarMesh->GetChildByName(str("%d", 20)); // rear left
	pCarWheels[3] = pCarMesh->GetChildByName(str("%d", 17)); // rear right	

	// Process wheel geometry
	for (int i=0; i<4; i++)
	{
		CObject3D *wheel = pCarWheels[i];	
		wheel->physicsDelay = 20.0f;
		wheel->position = Vector3();		// reset position
		pCarMesh->Remove(wheel);				// remove from original body
		wheel->geometry->materials[0]->specular = 0.2f;
		((CMesh*)wheel)->Reposition();		// center geometry around (0,0,0)
		wheel->SetScale(carContainer->GetScale());			
	}

	// Set relative positions of the wheels
	pCarWheels[0]->SetPosition(-carSize.x/2 - carWheelOffset.x, carWheelOffset.y, carSize.z/2 - carWheelOffset.z);
	pCarWheels[1]->SetPosition(+carSize.x/2 + carWheelOffset.x, carWheelOffset.y, carSize.z/2 - carWheelOffset.z);
	pCarWheels[2]->SetPosition(-carSize.x/2 - carWheelOffset.x, carWheelOffset.y, -carSize.z/2 + carWheelOffset.z);
	pCarWheels[3]->SetPosition(+carSize.x/2 + carWheelOffset.x, carWheelOffset.y, -carSize.z/2 + carWheelOffset.z);


	// Create car joint
	float m[16]; 	
	CArray<Vector3> carVertices;
	pCarMesh->GetAllVertices(carVertices);
	int carPointsNum = carVertices.Size();
	Vector3 *carPoints = new Vector3[carPointsNum];

	NewtonWorld *pWorld = pGame->pWorld;

	carContainer->UpdateMatrixWorld(true);
	for (int i=0; i<carPointsNum; i++)	
		carPoints[i] = Vector3(pCarMesh->matrixWorld.MultiplyVector3(carVertices[i]));

	// TODO: real car mesh collision hull
	NewtonCollision *pCarCollision = NewtonCreateConvexHull(pGame->pWorld, carPointsNum, &carPoints[0][0], 12, 0.001f, 0, NULL);
	delete [] carPoints;
	pCarPhysBody = CPhysics::CreateBox(pWorld, this, carSize.x, carSize.y, carSize.z, 10000);
	NewtonBodySetForceAndTorqueCallback(pCarPhysBody, BoxGravityCallback);
	
	Vector3 origin, inertia, minBox, maxBox;
	carContainer->matrixModel.FlattenToArray(m);
	NewtonCollision *chassisCollision;

	// calculate the moment of inertia and the relative center of mass of the solid
	chassisCollision = NewtonBodyGetCollision(pCarPhysBody);
	NewtonConvexCollisionCalculateInertialMatrix (chassisCollision, &inertia[0], &origin[0]);
	CalculateNewtonBoundingBox(chassisCollision, minBox, maxBox);

	//displace the center of mass by some value
	origin.y += 0.5f * (maxBox.y - minBox.y) * 0.75f;

	// setup materials
	int levelId = gTerrain.materialId;
	static int carId = NewtonMaterialCreateGroupID(pWorld);

	NewtonMaterialSetDefaultCollidable(pWorld, levelId, carId, 1);	
	NewtonMaterialSetDefaultFriction(pWorld, levelId, carId, 1.0f, 1.0f);
	NewtonMaterialSetDefaultSoftness(pWorld, levelId, carId, 0.0f);
	NewtonMaterialSetDefaultElasticity(pWorld, levelId, carId, 0.5f);

	// assign materials
	NewtonBodySetMaterialGroupID(pCarPhysBody, carId);


	wheelAngle = 0;

	// MultiBody Vehicle
	Matrix4 chassisMatrix;	
	chassisMatrix.SetRotationY(-90 * ToRad);			// orient the chassis to (1,0,0) as front
	chassisMatrix.FlattenToArray(m);
	Vector3 carFront(0,0,1);				// 0,0,1 OK	
	Vector3 carUp(0,1,0);					// 0,1,0 OK	
	pCarController = CreateCustomMultiBodyVehicle(&carFront[0], &carUp[0], pCarPhysBody);

	float suspensionLength = 0.35f;
	float suspensionConst = 100.0f;
	float suspensionDampening = 5.0f;

	// create tires, add transform callback so they are positioned correctly
	for (int i=0; i<4; i++)
	{
		CustomMultiBodyVehicleAddTire(pCarController, pCarWheels[i], &pCarWheels[i]->position[0], 500.0f, carWheelRadius, carWheelThickness, suspensionLength, suspensionConst, suspensionDampening);		
		const NewtonBody * tire = CustomMultiBodyVehicleGetTireBody( pCarController, i );
		NewtonBodySetTransformCallback(tire, CPhysics::TransformCallback);
		NewtonBodySetDestructorCallback(tire, CPhysics::DestroyBodyCallback);
		NewtonBodySetMaterialGroupID(tire, carId);
	}

	// never sleep
	NewtonBodySetAutoSleep(pCarPhysBody, 0);
}

CCarEntity::~CCarEntity()
{

}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::Update( float frametime )
{
	CEntity::Update(frametime);

	if (velocity.Length() > 100)
		Assert(true);
}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::AimAt(Vector3 target, float frametime)
{
	static float minBarrelAngle = 20;

	float turretRotation = pBarrel->rotation.z;
	float barrelRotation = pBarrel->rotation.x;

	float turretRotationSpeed = 180;
	float barrelRotationSpeed = 150;


	target = this->ToLocalVector(target);

	Vector3 current = pBarrel->rotation;

	pBarrel->LookAt(target);
	Vector3 targetRot = pBarrel->rotation;

	// Limit the pitch of the barrel
	if (current.x < 270 && current.x > minBarrelAngle) current.x = minBarrelAngle;


	StepAngle(current.y, targetRot.y, turretRotationSpeed, frametime);
	StepAngle(current.x, targetRot.x, barrelRotationSpeed, frametime);

	pBarrel->SetRotation(current);
	pTurret->SetRotationZ(-pBarrel->rotation.y);
}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::AddToScene( CScene *pScene )
{
	for (int i=0; i<4; i++) pScene->Add(pCarWheels[i]);
}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::ControlCar( float frametime )
{
	// DEBUG: draw car bbox
	//gRenderer.AddBBox(pCarMesh->bbox, pCarMesh->GetPosition(), RED, &pCarMesh->matrixWorld);

	// show some car parameters
	Vector3 speed;	
	NewtonBodyGetVelocity(pCarPhysBody, &speed[0]);

	// Steering
	if (Keydown('a'))
	{
		wheelAngle -= cv_angle.GetFloat() * frametime * 5.0f;
	}
	else if (Keydown('d'))
	{
		wheelAngle += cv_angle.GetFloat() * frametime * 5.0f;
	}

	// Check for up-side-down 
	static float upSideDownCounter = 0;
	static float resetTimeout = 0;			// how much time passed since reset
	Vector3 bodyRotation = this->up;

	// force reposition of R is pressed
	if (KeyPressed('r'))
	{
		Reset();
		resetTimeout = 0.5f;		// half a second
		wheelAngle = 0;
		upSideDownCounter = 0;
	}

	if (bodyRotation.y > 0 || speed.Length() > 2.0f) 
	{
		upSideDownCounter = 0;
	}
	else
	{
		upSideDownCounter += frametime;
		if (upSideDownCounter > 3)		
		{
			Reset();
			resetTimeout = 0.5f;		// half a second
			wheelAngle = 0;
			upSideDownCounter = 0;
		}
	}
	

	// If not diring reset position timeout, perform car controls
	if (resetTimeout > 0) 
	{
		// If we just reset the position of the car, do not apply sterring
		resetTimeout -= frametime;
	}
	else
	{
		// Brake
		if (Keydown('z') || Keydown(K_SPACE))
		{		
			for (int i=0; i<4; i++)
				CustomMultiBodyVehicleApplyBrake(pCarController, i, cv_brake.GetFloat());
		}
		else
		{
			float speed = cv_torque.GetFloat();
			if (Keydown(K_SHIFT)) speed += cv_boost.GetFloat();

			// controlling the car
			if (Keydown('w'))
			{
				ApplyTorque(speed);			
			}

			if (Keydown('s'))
			{
				ApplyTorque(-speed);			
			}
		}

		// We can apply steering
		if (wheelAngle < 0) 
		{
			wheelAngle +=  cv_angle.GetFloat() * frametime * 1.5f;
			if (wheelAngle > 0) wheelAngle = 0;
		}
		else if (wheelAngle > 0) 
		{
			wheelAngle -=  cv_angle.GetFloat() * frametime * 1.5f;
			if (wheelAngle < 0) wheelAngle = 0;
		}

		float damp = max(1.0f, (speed.Length() / cv_angleDampening.GetFloat()));
		wheelAngle = max(-cv_angle.GetFloat() / damp, wheelAngle);
		wheelAngle = min(cv_angle.GetFloat() / damp, wheelAngle);

		CustomMultiBodyVehicleApplySteering(pCarController, 0, wheelAngle);
		CustomMultiBodyVehicleApplySteering(pCarController, 1, wheelAngle);
	}
}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::Reset(Vector3 *pos)
{
	// restabilize car
	dMatrix mat;
	NewtonBodyGetMatrix(pCarPhysBody, &mat[0][0]);

	Vector3 location = pos ? *pos : Vector3(mat.m_posit.m_x, mat.m_posit.m_y, mat.m_posit.m_z);

	// move the car to the new location so we dont zapin the physics callback
	SetPosition(location);
	for (int i=0; i<4; i++)
		pCarWheels[i]->SetPosition(location);

	mat.m_posit.m_x = location.x;
	mat.m_posit.m_y = location.y;
	mat.m_posit.m_z = location.z;

	mat.m_posit.m_y += 1.0f;
	mat.m_front = dVector(0,0,1);
	mat.m_up = dVector(0,1,0);
	mat.m_right = dVector(1,0,0);

	float v[3] = { 0, 0, 0 };			
	NewtonBodySetOmega(pCarPhysBody, v);
	NewtonBodySetVelocity(pCarPhysBody,v);
	NewtonBodySetTorque(pCarPhysBody, v);
	NewtonBodySetMatrix(pCarPhysBody, &mat[0][0]);

	float x = mat.m_posit.m_x;
	float y = mat.m_posit.m_y;
	float z = mat.m_posit.m_z;

	CustomMultiBodyVehicleApplySteering(pCarController, 0, 0);
	CustomMultiBodyVehicleApplySteering(pCarController, 1, 0);

	for (int i=0; i<4; i++)
	{
		const NewtonBody * tire = CustomMultiBodyVehicleGetTireBody( pCarController, i );
		NewtonBodyGetMatrix(tire, &mat[0][0]);
		mat.m_posit.m_y = y;
		mat.m_front = dVector(0,0,1);
		mat.m_up = dVector(0,1,0);
		mat.m_right = dVector(1,0,0);				
		CustomMultiBodyVehicleApplyTorque (this->pCarController, i, 0);
		NewtonBodySetOmega(tire, v);
		NewtonBodySetVelocity(tire,v);
		NewtonBodySetTorque(tire, v);
		NewtonBodySetMatrix(tire, &mat[0][0]);
	}

	wheelAngle = 0;
}

//////////////////////////////////////////////////////////////////////////
void CCarEntity::ApplyTorque( float value )
{
	Vector3 vspeed;	
	NewtonBodyGetVelocity(pCarPhysBody, &vspeed[0]);
	float speed = vspeed.Length();

	// torque should be small when starting but pick up when already have speed
	// full torque used when speed > torqueStart
	float torqueStartSpeed = cv_torqueStart.GetFloat();

	value = min(1.0f, (speed / torqueStartSpeed) / 2.0f + 0.5f) * value;

	CustomMultiBodyVehicleApplyTorque (this->pCarController, 0, -value);
	CustomMultiBodyVehicleApplyTorque (this->pCarController, 1, -value);
	CustomMultiBodyVehicleApplyTireRollingDrag(this->pCarController, 0, cv_drag.GetFloat());
	CustomMultiBodyVehicleApplyTireRollingDrag(this->pCarController, 1, cv_drag.GetFloat());
}