#pragma once
#include "platform.h"
#include "Vector.h"
#include "Array.h"
#include "Utils.h"
#include "Matrix.h"
#include "Utils3D.h"

struct Matrix4;
class CMaterial;
class CGeometry;

//////////////////////////////////////////////////////////////////////////
namespace EBoundingShape { enum Type 
{
	Box = 0,
	Sphere
}; };

//////////////////////////////////////////////////////////////////////////
class CObject3D
{
public:
	CObject3D();
	virtual ~CObject3D(void);

	TYPE("Object3D");

	CObject3D*		Copy();

	void			SetRotationX(float angle);
	void			SetRotationY(float angle);
	void			SetRotationZ(float angle);

	void			SetPosition(float x, float y, float z);
	void			SetRotation(float x, float y, float z);
	void			SetVelocity(float x, float y, float z);
	void			SetScale(float x, float y, float z);
	void			SetScale(float x);
	void			SetScale(Vector3 &s);
	void			ScaleUp(float x, float y, float z);
	void			ScaleUp(float x);

	void			SetPosition(Vector3 &pos);
	void			SetRotation(Vector3 &rot);

	void			Move(float x, float y, float z);
	void			Move(Vector3 &v);
	void			Rotate(float x, float y, float z);
	void			Rotate(Vector3 &v);

	float			DistanceTo(CObject3D *obj);
	float			DistanceTo(Vector3 &pos);

	void			MoveForward(float distance);
	void			MoveRight(float distance);
	void			MoveUp(float distance);
	void			LookAt(Vector3 target);
	void			LookAt(float x, float y, float z);
	void			SetMatrix(Matrix4 &matrix);
	void			SetDirection(Vector3 &dir);
	void			MoveTo(Vector3 dest, float distance);
	void			ScaleToFit(float x, float y, float z);

	Vector3			GetWorldVector(Vector3 vec);
	Vector3			ToLocalVector(Vector3 vec);

	virtual void	Update(float frametime);

	inline Vector3	GetPosition() { return position; }
	inline Vector3	GetWorldPosition() { return matrixWorld.GetPosition(); }
	inline Vector3	GetRotation() { return rotation; }
	inline Vector3	GetVelocity() { return velocity; }
	inline Vector3	GetForwardVector() { return forward; }
	inline Vector3	GetRightVector() { return right; }
	inline Vector3	GetUpVector() { return up; }
	inline Vector3	GetScale() { return scale; }

	bool			LineBoundingSphereIntersection(Vector3 start, Vector3 end, OUT Vector3 &hitPos);
	bool			LineIntersection(Vector3 start, Vector3 end, bool findClosest = false, float* t = NULL);

	void			CalculateBoundingShape(Matrix4 matrix = Matrix4());
	void			DrawBoundingBox(bool recursive = false, SRGB color = RED);
	void			DrawBoundingSphere(bool recursive = false, SRGB color = RED);
	
	void			NormalizeGeometry(float sizeX, float sizeY, float Z);

	void			Add(CObject3D* object);
	bool			Remove(CObject3D* object);
	void			Empty();
	CObject3D*		GetChildByName(string name, bool doRecurse = true);
	void			UpdateMatrix();
	void			UpdateMatrixWorld(bool force = false);

	void			Reposition();	
	float			GetBoudingSphereRadius();	
	void			DrawAxis();

	void			GetAllVertices(CArray<Vector3> &ret);

protected:

	virtual void	CalculateVectors();
	virtual void	ValidateAngles();

public:
	static uint32	nextId;			// unique ID sequence

	string		name;
	uint32		id;
	bool		deleteMe;			// object has been marked for deletion

	CObject3D			*parent;
	CArray<CObject3D*>	children;

	Vector3		position;
	Vector3		rotation;
	Vector3		velocity;	
	Vector3		acceleration;
	Vector3		forward, right, up;
	Vector3		scale;	

	float		zOffset;
	float		opacity;

	bool		matrixAutoUpdate;
	bool		rotationAutoUpdate;
	bool		updateMatrix;				// does the matrix need updating?

	bool		dynamic;					// when set the geometry array is kept in memory so it can be modified
	bool		visible;					// visible in rendering?
	
	bool		castShadow;					// cast shadow?
	bool		receiveShadow;				// receive shadows from casters?
	bool		frustumCulled;				// use frustum culling on the object?

	Vector4		quaternion;
	bool		useQuaternion;
	string		eulerOrder;
	float		boundingRadiusScale;
	bool		noHitTest;					// object not hittable (unless in select mode)

	SRGB		color;

	CMaterial	*material;
	CGeometry	*geometry;
		
	float		physicsDelay;				// how quickly objects catches up with physics state
	float		killTime;

	SBBox						bbox;				// bbox of entire model
	EBoundingShape::Type		boundingShape;		// shape used for frustum/collisions
	

public:
	Matrix4		matrixModel;				// local transformations
	Matrix4		matrixWorld;				// world space matrix generated every frame from parents transforms
	
protected:
	bool		matrixWorldNeedsUpdate;
};