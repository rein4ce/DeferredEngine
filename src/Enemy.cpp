#include "platform.h"
#include "Enemy.h"
#include "Geometries\CubeGeometry.h"
#include "Geometries\SphereGeometry.h"
#include "TestGame.h"
#include "Car.h"
#include "Renderer.h"
#include "Engine.h"
#include "ModelLoader.h"

CEnemy::CEnemy()
{
	CObject3D *model = CModelLoader::Get("models/robo.dae");
	model->SetScale(0.01f);
	model->SetRotationX(-90);
	model->SetRotationY(90);
	model->SetPosition(0,-0.2,0);
	this->Add(model);
	color = SRGBA(155,155,155,255);
	this->destructible = true;
	lastWaypoint = NULL;
	pTarget = NULL;
	lastShot = 0;
}

CEnemy::~CEnemy()
{

}

int CEnemy::operator()( CEntity *a,  CEntity *b)
{
	if (!a || !b) return 0;
	return (DistanceTo(a) < DistanceTo(b));
}

void CEnemy::Update( float frametime )
{
	CEntity::Update(frametime);

	if (hp <= 0)
	{
		((CTestGame*)pGame)->CreateSmallExplosion(GetWorldPosition());
		deleteMe = true;
		return;
	}

	CEntity *car = ((CTestGame*)pGame)->pCar;
	float distance = car->DistanceTo(this);
	if (distance < 15)
	{
		// We are in range
		color = YELLOW;
		LookAt(car->GetWorldPosition());

		pTarget = NULL;		// forget about the waypoints

		// if we are still to far away, come closer
		if (distance > 8)
		{
			velocity = (car->GetWorldPosition() - GetWorldPosition()).Normalize() * 1.5f; 
		}
		else
		{
			color = RED;
			velocity = Vector3();		// stop

			// shoot
			if (gEngine.realtime > lastShot + 1)
			{
				color = BLACK;
				gRenderer.AddLine(GetWorldPosition(), car->GetWorldPosition(), RED);
				lastShot = gEngine.realtime;
				((CTestGame*)pGame)->hp -= 5;
			}
		}
	}
	else
	{
		// Not in range, proceed to the waypoint
		color = WHITE;
		velocity = Vector3();

		if (pTarget && pTarget->GetType() == "Car") pTarget = NULL;

		// If moving towards a waypoint
		if (pTarget && DistanceTo(pTarget) > 0.5f)
		{
			LookAt(pTarget->GetWorldPosition());
			velocity = (pTarget->GetWorldPosition() - GetWorldPosition()).Normalize();
		}
		else
		{
			// Find closest waypoints
			CArray<CWaypoint*> closest = CArray<CWaypoint*>();
			for (int i=0; i<((CTestGame*)pGame)->waypoints.Size(); i++)
				if (DistanceTo(((CTestGame*)pGame)->waypoints[i]) < 7 && pTarget != ((CTestGame*)pGame)->waypoints[i] && ((CTestGame*)pGame)->waypoints[i] != lastWaypoint)
					closest.Add((CWaypoint*)((CTestGame*)pGame)->waypoints[i]);
			

			lastWaypoint = pTarget;

			// If there ano waypoints in proximity, just choose any 
			if (closest.Size() == 0)
			{				
				int rnd = rand() % ((CTestGame*)pGame)->waypoints.Size();
				pTarget = ((CTestGame*)pGame)->waypoints[rnd];
			}
			else
			{
				int rnd = rand() % closest.Size();
				pTarget = closest[rnd];
			}
		}	
	}	
}

CWaypoint::CWaypoint()
{
	geometry = new CSphereGeometry(0.3f);
	color = GREEN;
	noHitTest = true;
	serialized = true;
	editorOnly = true;
}