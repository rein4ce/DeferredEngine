#pragma once
#include "platform.h"
#include "Utils.h"
#include "Sprite.h"
#include "Object3D.h"
#include "SlotArray.h"
#include "Vector.h"
#include "Array.h"

class CObject3D;
class CParticle;
class CScene;
class CEntity;

typedef void (*ParticleUpdateFn)(CParticle*);

struct SRange
{
	float min, max;
	SRange()
	{
		min = max = 0;
	}
	SRange(float a)
	{
		min = max = a;
	}
	SRange(float a, float b)
	{
		min = a;
		max = b;
	}
	float Get()
	{
		if (min == max) return min;
		return frand(min, max);
	}
};

//////////////////////////////////////////////////////////////////////////
struct SParticleEffectInfo
{
	SRange				count;					// how many to spawn
	SRange				time;					// time offset to spawn
	string				texture;				// texture name
	SRange				posX, posY, posZ;		// relattive position
	SRange				velX, velY, velZ;		// velocity
	SRange				accX, accY, accZ;		// acceleration
	SRange				sizeX, sizeY;			// size of the sprite
	SRange				sizeVelX, sizeVelY;		// change of size over time
	SRange				rotation;				// rotation
	SRange				rotationVel;			// rotation velocity
	SRange				r, g, b, a;				// color
	SRange				er, eg, eb, ea;			// end color
	SRange				lifetime;				// how long will it live
	bool				additive;				// additive?

	SParticleEffectInfo()
	{		
		count = SRange(1);
		sizeX = SRange(1);
		sizeY = SRange(1);
		r = SRange(255);
		g = SRange(255);
		b = SRange(255);
		a = SRange(255);
		er = SRange(255);
		eg = SRange(255);
		eb = SRange(255);
		ea = SRange(255);
	}

	void Spawn(Vector3 pos);
};

//////////////////////////////////////////////////////////////////////////
class CParticleEffect
{
public:
	void				Spawn(Vector3 position);

public:
	string							name;
	CArray<SParticleEffectInfo>		particles;
};

//////////////////////////////////////////////////////////////////////////
class CParticleEffectsManager
{
public:
	void				Init();
	void				Release();
	void				Update();
	CParticleEffect*	Get(string name);
	CEntity*			Create(string name, Vector3 pos);

private:
	SRange				ParseRangeValue(string value);

private:
	map<string, CParticleEffect*>	effects;
};

//////////////////////////////////////////////////////////////////////////
class CParticle : public CSprite
{
public:
	Vector3				velocity;			// velocity
	Vector2				sizeVel;			// speed of size change
	float				rotationVel;		// speed of rotation change
	Vector3				acceleration;		// how the speed changes over time

	SRGBA				colorChange;		// to what color we change (difference between colors)
	float				colorTime;			// after what time (if 0 - no color change)
	float				lifetime;			// lifetime = 0 means don't kill
	float				killtime;			// when to kill

	float				gravity;
	
	ParticleUpdateFn	updateCallback;		// additional update callback

public:
	float				colorEnd;
	sint16				colorStep[4];		// how each color component changes

	bool				deleteMe;			// just remove the particle

public:
	CParticle(CTexture *tex, CObject3D *pParent = NULL);	
};

//////////////////////////////////////////////////////////////////////////
// A system that manages a certain pool of particles, updating them,
// releasing etc. Because particle system is an object, it can be
// positioned, scaled, rotatted, overriden with color etc
//////////////////////////////////////////////////////////////////////////
class CParticleSystem : public CObject3D
{
public:
	CParticleSystem(CScene *pScene, int maxParticles = 1024);
	virtual ~CParticleSystem();

	void				Init(int maxParticles);
	void				Reset();

	void				Add(CParticle *particle);
	void				Remove(CParticle *particle);

	void				Update(float frametime, float realtime);
	void				Render(float frametime, float realtime);

	inline int			GetCount() { return particles.count; }

public:
	CSlotArray<CParticle>	particles;

	CScene					*pScene;
};

extern CParticleEffectsManager gEffects;