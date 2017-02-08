#include "platform.h"
#include "Utils.h"
#include "Object3D.h"
#include "Scene.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Collision.h"
#include "VGUI.h"
#include "Game.h"
#include "CVar.h"
#include "Editor.h"

uint32 CObject3D::nextId = 0;			// global ID for objects

CObject3D::CObject3D() 
{
	position = Vector3(0,0,0);
	rotation = Vector3(0,0,0);
	velocity = Vector3(0,0,0);
	forward = Vector3(0,0,1);
	right = Vector3(1,0,0);
	up = Vector3(0,1,0);
	scale = Vector3(1,1,1);
	id = nextId++;

	killTime = 0.0;

	name = "";
	parent = NULL;	
	deleteMe = false;

	material = NULL;
	geometry = NULL;

	visible = true;
	dynamic = false;	
	matrixAutoUpdate = false;
	rotationAutoUpdate = true;
	matrixWorldNeedsUpdate = true;
	castShadow = true;
	receiveShadow = false;
	frustumCulled = true;
	updateMatrix = true;
	useQuaternion = false;
	quaternion = Vector4();
	eulerOrder = "ZYX";
	boundingRadiusScale = 1.0f;
	zOffset = 0;
	opacity = 1.0f;	
	noHitTest = false;

	boundingShape = EBoundingShape::Sphere;

	physicsDelay = 10.0f;

	color = WHITE;

	matrixModel = Matrix4();
	matrixWorld = Matrix4();			// filled by UpdateMatrix() with position, scale and rotation
}

CObject3D::~CObject3D() 
{
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::SetPosition(float x, float y, float z)
{
	position = Vector3(x,y,z);
	updateMatrix = true;
}

void CObject3D::SetRotation(float x, float y, float z)
{
	rotation = Vector3(x,y,z);
	ValidateAngles();
	CalculateVectors();
}

void CObject3D::SetVelocity(float x, float y, float z)
{
	velocity = Vector3(x,y,z);
}

void CObject3D::SetScale(float x, float y, float z)
{
	scale = Vector3(x,y,z);
	updateMatrix = true;
}

void CObject3D::SetScale(Vector3 &s)
{
	scale = s;
	updateMatrix = true;
	boundingRadiusScale = max(scale.x, max(scale.y, scale.z));		
	if (parent) boundingRadiusScale = boundingRadiusScale * parent->boundingRadiusScale;
}

void CObject3D::SetScale(float x)
{
	scale = Vector3(x,x,x);
	updateMatrix = true;
	boundingRadiusScale = max(scale.x, max(scale.y, scale.z));		
	if (parent) boundingRadiusScale = boundingRadiusScale * parent->boundingRadiusScale;
}

void CObject3D::ScaleUp(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	updateMatrix = true;
	boundingRadiusScale = max(scale.x, max(scale.y, scale.z));		
	if (parent) boundingRadiusScale = boundingRadiusScale * parent->boundingRadiusScale;
}

void CObject3D::ScaleUp(float x)
{
	scale *= x;
	updateMatrix = true;
	boundingRadiusScale = max(scale.x, max(scale.y, scale.z));		
	if (parent) boundingRadiusScale = boundingRadiusScale * parent->boundingRadiusScale;
}


//////////////////////////////////////////////////////////////////////////
void CObject3D::SetPosition(Vector3 &pos)
{
	SetPosition(pos.x, pos.y, pos.z);
}

void CObject3D::SetRotation(Vector3 &rot)
{
	SetRotation(rot.x, rot.y, rot.z);
}


//////////////////////////////////////////////////////////////////////////
void CObject3D::Move(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	updateMatrix = true;
}

void CObject3D::Move(Vector3 &v)
{
	position += v;
	updateMatrix = true;
}

void CObject3D::Rotate(float x, float y, float z)
{
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
	ValidateAngles();
	CalculateVectors();
}

void CObject3D::Rotate(Vector3 &v)
{
	rotation += v;
	ValidateAngles();
	CalculateVectors();
}

void CObject3D::SetRotationX(float angle)
{
	rotation.x = angle;
	ValidateAngles();
	CalculateVectors();
}

void CObject3D::SetRotationY(float angle)
{
	rotation.y = angle;
	ValidateAngles();
	CalculateVectors();
}

void CObject3D::SetRotationZ(float angle)
{
	rotation.z = angle;
	ValidateAngles();
	CalculateVectors();
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::MoveForward(float distance)
{
	Vector3 vec = distance * forward;
	position += vec;
	updateMatrix = true;
}

void CObject3D::MoveRight(float distance)
{
	Vector3 vec = distance * right;
	position += vec;
	updateMatrix = true;
}

void CObject3D::MoveUp(float distance)
{
	Vector3 vec = distance * up;
	position += vec;
	updateMatrix = true;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::ValidateAngles()
{
	int i = 0;
 	if ( rotation.x < 0 ) { i = floor(rotation.x / 360.0f); rotation.x -= i * 360.0f; }
 	if ( rotation.y < 0 ) { i = floor(rotation.y / 360.0f); rotation.y -= i * 360.0f; }
 	if ( rotation.z < 0 ) { i = floor(rotation.z / 360.0f); rotation.z -= i * 360.0f; }
	if ( rotation.x > 360.0f ) { i = floor(rotation.x / 360.0f); rotation.x -= i * 360.0f; }
	if ( rotation.y > 360.0f ) { i = floor(rotation.y / 360.0f); rotation.y -= i * 360.0f; }
	if ( rotation.z > 360.0f ) { i = floor(rotation.z / 360.0f); rotation.z -= i * 360.0f; }
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::Update(float frametime)
{
	velocity += acceleration * frametime;	

	if (velocity.LengthSq() != 0)
	{
		Vector3 move = velocity * frametime;
		Move(move.x, move.y, move.z);				
	}
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateVectors()
{
	if (useQuaternion)
		matrixModel.SetRotationFromQuaternion(quaternion);
	else
		matrixModel.SetRotationFromEuler(rotation, eulerOrder);

	this->updateMatrix = true;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::Add(CObject3D* object )
{
	Assert(object && this->id != object->id);
	if (children.Find(object) == -1) 
	{
		if (object->parent != NULL)
			object->parent->Remove(object);

		object->parent = this;
		children.AddToTail(object);

		// add to parent scene
		CObject3D *scene = this;
		while (scene && scene->parent) scene = scene->parent;

		if (scene != NULL && scene->GetType() == "Scene")
			((CScene*)scene)->AddObject(object);
	}
}

//////////////////////////////////////////////////////////////////////////
// Mark child object and it's children for deletion
//////////////////////////////////////////////////////////////////////////
bool CObject3D::Remove( CObject3D* object )
{
	Assert(object);
	object->deleteMe = true;
	int index = children.Find(object);

	if (index != -1)
	{
		object->parent = NULL;
		children.RemoveAt(index);

		// remove from parent scene
		CObject3D *scene = this;
		while (scene->parent) scene = scene->parent;

		if (scene != NULL && scene->GetType() == "Scene")
			((CScene*)scene)->RemoveObject(object);

		return true;
	}
	else
	{
		for (int i=0; i<children.Size(); i++)
		{
			if (children[i]->Remove(object)) return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::Empty()
{
	// remove from parent scene
	CObject3D *scene = this;
	while (scene->parent) scene = scene->parent;

	for (int i=0; i<children.Size(); i++)
	{
		CObject3D *child = children[i];
		child->parent = NULL;
		children.RemoveAt(i--);		
		child->deleteMe = true;
		
		if (scene != NULL && scene->GetType() == "Scene")
			((CScene*)scene)->RemoveObject(child);
	}
}

//////////////////////////////////////////////////////////////////////////
CObject3D* CObject3D::GetChildByName( string name, bool doRecurse /*= true*/ )
{
	CObject3D *child, *recurseResult;
	for (int i=0; i<children.Size(); i++)
	{
		child = children[i];
		if (child->name == name) return child;

		if (doRecurse)
		{
			recurseResult = child->GetChildByName(name, doRecurse);
			if (recurseResult != NULL) return recurseResult;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::UpdateMatrix()
{
	matrixModel.SetPosition(position);

	if (useQuaternion)
		matrixModel.SetRotationFromQuaternion(quaternion);
	else
		matrixModel.SetRotationFromEuler(rotation, eulerOrder);
	
	// Update bouding boxes
	if (scale.x != 1 || scale.y != 1 || scale.z != 1)
	{
		matrixModel.Scale(scale);			
	}
	boundingRadiusScale = max(scale.x, max(scale.y, scale.z));		
	if (parent) boundingRadiusScale = boundingRadiusScale * parent->boundingRadiusScale;

	matrixWorldNeedsUpdate = true;
	updateMatrix = false;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::UpdateMatrixWorld(bool force /* = false */)
{
	if (matrixAutoUpdate || updateMatrix) UpdateMatrix();

	if (matrixWorldNeedsUpdate || force) 
	{
		if (parent)
			matrixWorld = Matrix4::Multiply( parent->matrixWorld, matrixModel );
		else
			matrixWorld.Set(matrixModel);

		right = matrixWorld.GetColumnX().Normalize();
		up = matrixWorld.GetColumnY().Normalize();
		forward = matrixWorld.GetColumnZ().Normalize();

		matrixWorldNeedsUpdate = false;
		force = true;
	}

	// update children
	for (int i=0; i<children.Size(); i++)
		children[i]->UpdateMatrixWorld(force);
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::LookAt( Vector3 target )
{
	Matrix4 m = matrixModel.LookAt( position, target, Vector3(0,1,0) );	
	
	if (rotationAutoUpdate)
	{
		rotation.SetRotationFromMatrix(m);
		rotation *= ToDeg;
		SetRotation(rotation);
	}
	updateMatrix = true;
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::LookAt(float x, float y, float z)
{
	LookAt(Vector3(x,y,z));
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::SetMatrix( Matrix4 &matrix )
{
	matrixModel = matrix;
	position = matrix.GetPosition();
	rotation = matrixModel.GetEulerAngles() * ToDeg;
	updateMatrix = true;
	matrixWorldNeedsUpdate = true;
}

//////////////////////////////////////////////////////////////////////////
float CObject3D::GetBoudingSphereRadius()
{
	if (!geometry) return 0;
	return abs(geometry->boundingSphere * boundingRadiusScale);
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::DrawAxis()
{
	Vector3 pos = GetWorldPosition();
	gRenderer.AddLine(pos, pos+up, BLUE);
	gRenderer.AddLine(pos, pos+right, RED);
	gRenderer.AddLine(pos, pos+forward, GREEN);
}

//////////////////////////////////////////////////////////////////////////
bool CObject3D::LineIntersection( Vector3 start, Vector3 end, bool findClosest, float* t )
{
	if (!geometry) return false;
	start -= GetWorldPosition();
	end -= GetWorldPosition();
	float minT = 9999.0f;
	float T = 0;
	for (int i=0; i<geometry->faces.Size(); i++)
		if (CCollision::LineTriangle(geometry->vertices[geometry->faces[i].a], geometry->vertices[geometry->faces[i].b], geometry->vertices[geometry->faces[i].c], start, end, &T))
		{
			if (T < minT)
			{
				minT = T;
				*t = T;
			}
			if (!findClosest) return true;
		}

	if (findClosest && minT < 9999.0f) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
CObject3D* CObject3D::Copy()
{
	CObject3D* ret = new CObject3D(*this);
	ret->id = CObject3D::nextId++;

	ret->children = CArray<CObject3D*>();

	for (int i=0; i<children.Size(); i++)
	{
		CObject3D *child = children[i]->Copy();
		child->parent = ret;
		ret->children.AddToTail(child);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Add world positions of bbox points for this object and it's children
//////////////////////////////////////////////////////////////////////////
void GetBoundingBoxWorldPoints(CObject3D *object, CArray<Vector3> &points, bool recursive = false)
{
	if (!object) return;

	// coordinates of child bbox in world space
	Vector3 p[8] = 
	{
		object->bbox.min,
		Vector3(object->bbox.min.x, object->bbox.min.y, object->bbox.max.z),
		Vector3(object->bbox.max.x, object->bbox.min.y, object->bbox.min.z),
		Vector3(object->bbox.min.x, object->bbox.max.y, object->bbox.min.z),
		object->bbox.max,
		Vector3(object->bbox.max.x, object->bbox.max.y, object->bbox.min.z),
		Vector3(object->bbox.min.x, object->bbox.max.y, object->bbox.max.z),
		Vector3(object->bbox.max.x, object->bbox.min.y, object->bbox.max.z)
	};

	// convert points to world space
	for (int i=0; i<8; i++)
	{
		p[i] = object->matrixWorld.MultiplyVector3(p[i]);
		points.AddToTail(p[i]);
	}

	// children
	if (recursive)
		for (int i=0; i<object->children.Size(); i++)
			GetBoundingBoxWorldPoints(object->children[i], points, true);
}

//////////////////////////////////////////////////////////////////////////
// Scale entire geometry (including children geometry) to fit inside
// specified area
//////////////////////////////////////////////////////////////////////////
void CObject3D::NormalizeGeometry( float sizeX, float sizeY, float sizeZ )
{
	// calculate bboxes of this object and it's children
	CalculateBoundingShape();

	// get all world positions of bboxes
	CArray<Vector3> points;
	GetBoundingBoxWorldPoints(this, points, true);

	// calculate mins and maxes and get size
	Vector3 min = points[0];
	Vector3 max = points[0];

	for (int i=0; i<points.Size(); i++)
	{
		min.x = min(min.x, points[i].x);
		min.y = min(min.y, points[i].y);
		min.z = min(min.z, points[i].z);
		max.x = max(max.x, points[i].x);
		max.y = max(max.y, points[i].y);
		max.z = max(max.z, points[i].z);
	}

	Vector3 size = max - min;		// actual size of the object
	size.x -= sizeX;				// overflow beyond the limited size
	size.y -= sizeY;
	size.z -= sizeZ;

	// scale by the largest size difference
	if (size.x > size.y && size.x > size.z)
		this->ScaleUp(sizeX / (size.x + sizeX));
	else if (size.y > size.x && size.y > size.z)
		this->ScaleUp(sizeY / (size.y + sizeY));
	else if (size.z > size.x && size.z > size.y)
		this->ScaleUp(sizeZ / (size.z + sizeZ));
}



//////////////////////////////////////////////////////////////////////////
// Calculate bounding box including children geometry
// matrix - transformation matrix passed to children
//////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateBoundingShape(Matrix4 matrix)
{
	// update world matrix of this object and children
	UpdateMatrixWorld(true);

	if (geometry)
	{
		// if geometry, calculate bbox for geometry taking local transformation into account (scale, translation)
		geometry->ComputeBoundingShape();
		bbox = geometry->boundingBox;
		Vector3 vmin = (bbox.min);	//this->matrixModel.MultiplyVector3
		Vector3 vmax = (bbox.max);	//this->matrixModel.MultiplyVector3
		bbox.min = vmin;
		bbox.max = vmax;
	}
	else
	{
		bbox = SBBox();
	}
	

	for (int i=0; i<children.Size(); i++)
	{
		CObject3D *o = children[i];		
		o->CalculateBoundingShape(matrix * matrixModel);

		// calculated bbox of child has to be transformed to our coordinate system
		Vector3 vmin = (o->bbox.min);	//this->matrixModel.MultiplyVector3
		Vector3 vmax = (o->bbox.max);	//this->matrixModel.MultiplyVector3

		// coordinates of child bbox in current space
		Vector3 points[8] = {
			vmin,
			Vector3(vmin.x, vmin.y, vmax.z),
			Vector3(vmax.x, vmin.y, vmin.z),
			Vector3(vmin.x, vmax.y, vmin.z),
			vmax,
			Vector3(vmax.x, vmax.y, vmin.z),
			Vector3(vmin.x, vmax.y, vmax.z),
			Vector3(vmax.x, vmin.y, vmax.z)
		};

		// enlarge current bbox if necessary
		for (int j=0; j<8; j++)
		{
			bbox.min.x = min(bbox.min.x, points[j].x);
			bbox.min.y = min(bbox.min.y, points[j].y);
			bbox.min.z = min(bbox.min.z, points[j].z);

			bbox.max.x = max(bbox.max.x, points[j].x);
			bbox.max.y = max(bbox.max.y, points[j].y);
			bbox.max.z = max(bbox.max.z, points[j].z);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::DrawBoundingBox( bool recursive, SRGB color /*= RED*/ )
{
	gRenderer.AddBBox(bbox, GetWorldPosition(), color, &matrixWorld);

	// if recursive, draw children's bbox
	if (recursive)
		for (int i=0; i<children.Size(); i++)
			children[i]->DrawBoundingBox(true, color);
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::DrawBoundingSphere(bool recursive, SRGB color)
{
	gRenderer.Addsphere(GetWorldPosition(), GetBoudingSphereRadius(), color);
	if (recursive)
		for (int i=0; i<children.Size(); i++)
			children[i]->DrawBoundingSphere(true, color);
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::SetDirection( Vector3 &dir )
{
	LookAt(position + dir);
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::GetAllVertices( CArray<Vector3> &ret )
{
	if (geometry) ret.AddVectorToTail(geometry->vertices);
	for (int i=0; i<children.Size(); i++)
		children[i]->GetAllVertices(ret);
}

//////////////////////////////////////////////////////////////////////////
// Get world coordinates of vector within model space of this object
//////////////////////////////////////////////////////////////////////////
Vector3 CObject3D::GetWorldVector( Vector3 vec )
{
	return matrixWorld.MultiplyVector3(vec);
}

Vector3 CObject3D::ToLocalVector( Vector3 vec )
{
	return matrixWorld.GetInverse().MultiplyVector3(vec);
}

//////////////////////////////////////////////////////////////////////////
bool CObject3D::LineBoundingSphereIntersection( Vector3 start, Vector3 end, OUT Vector3 &hitPos)
{
	float fraction = 1.0f;

	// if object is excluded from hit tests
	if (noHitTest && !cv_editor.GetBool()) return false;
	
	float radius = boundingRadiusScale * (geometry ? geometry->boundingSphere : 1);

	// if inside editor, select by editor shape 
	if (gEditor.show && gEditor.mode == EDITOR_ENTITIES) radius = 1;

	if (geometry && CCollision::RaySphereIntersection(start, (end-start).Normalize(), GetWorldPosition(), radius, OUT fraction))
	{
		if (fraction <= (end-start).Length())
		{
			hitPos = start + (end-start).Normalize() * fraction;		
			return true;
		}
		return false;
	}
	else
	{
		// check children
		for (int i=0; i<children.Size(); i++)
		{
			if (children[i]->LineBoundingSphereIntersection(start, end, OUT hitPos))
				return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Center the geometry and use local tranformation to position it in 
// the original location
//////////////////////////////////////////////////////////////////////////
void CObject3D::Reposition()
{
	if (geometry)
	{
		Vector3 center = geometry->boundingBox.GetCenter();

		for (int j=0; j<geometry->vertices.Size(); j++)
			geometry->vertices[j] -= center;

		SetPosition(center);		
		geometry->dirtyVertices = true;
	}

	for (int i=0; i<children.Size(); i++)
		children[i]->Reposition();
}

//////////////////////////////////////////////////////////////////////////
float CObject3D::DistanceTo( CObject3D *obj )
{
	return DistanceTo(obj->GetWorldPosition());
}

float CObject3D::DistanceTo( Vector3 &pos )
{
	Vector3 p = pos - GetWorldPosition();
	return p.Length();
}

//////////////////////////////////////////////////////////////////////////
void CObject3D::MoveTo( Vector3 dest, float distance )
{
	Vector3 d = (dest - GetWorldPosition()).Normalize() * distance;
	Move(d);
}

//////////////////////////////////////////////////////////////////////////
// Scale mesh to fit inside the specified bounds
//////////////////////////////////////////////////////////////////////////
void CObject3D::ScaleToFit( float x, float y, float z )
{
	if (!geometry) return;
	geometry->ComputeBoundingShape();
	Vector3 size = geometry->boundingBox.GetSize();
	Vector3 factor = Vector3( x / size.x, y / size.y, z / size.z );
	float scale = min(factor.x, min(factor.y, factor.z));
	this->SetScale(scale);
}