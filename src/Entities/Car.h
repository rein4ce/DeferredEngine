#pragma once
#include "platform.h"
#include "Entity.h"

class NewtonUserJoint;
class NewtonBody;
class CObject3D;
class CScene;

class CCarEntity : public CEntity
{
	TYPE("Car");
public:
	CCarEntity();
	virtual ~CCarEntity();	

	void			ControlCar(float frametime);
	void			Reset(Vector3 *pos = NULL);
	void			ApplyTorque(float value);
	virtual void	Update(float frametime);
	void			AddToScene(CScene *pScene);
	void			AimAt(Vector3 target, float frametime);	

protected:	
	

public:
	NewtonUserJoint		*pCarController;		// controller joint
	NewtonBody			*pCarPhysBody;
	CObject3D			*pCarMesh;				// main car hull		
	CObject3D			*pCarWheels[4];			// wheels
	CObject3D			*pTurret;				// turret
	CObject3D			*pBarrel;				// minigun

	float				wheelAngle;				// steering angle
	

	Vector3			aimTarget;
	float			turretRotationTarget;
	float			barrelRotationTarget;
	float			turretRotationSpeed;
	float			barrelRotationSpeed;

	float			speed;
	float			acceleration;
	float			wheelOrientation;
	float			carOrientation;

	float			modelScale;
	float			wheelDiameter;

	float			maxSpeed;
	float			maxReverseSpeed;
	float			maxWheelRotation;
	float			frontAcceleration;
	float			backAcceleration;
	float			wheelAngularAcceleration;
	float			frontDecceleration;
	float			wheelAngularDecceleration;
	float			steeringRadiusRatio;
	float			maxTiltSides;
	float			maxTiltFrontBack;

	bool			wheelRotateZ;

	Vector3			intendedMove;
	Vector3			intendedRotation;

public:
	bool			moveForward;
	bool			moveBackward;
	bool			moveLeft;
	bool			moveRight;
};
