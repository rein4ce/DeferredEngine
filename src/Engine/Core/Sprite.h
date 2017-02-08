#pragma once
#include "platform.h"
#include "Object3D.h"
#include "Vector.h"
#include "Utils.h"

using namespace boost;

class CTexture;

//////////////////////////////////////////////////////////////////////////
// Simple 2d sprite positioned in 3d space (or the screen)
//////////////////////////////////////////////////////////////////////////
class CSprite 
{
	friend class CRenderer;

public:

	int				id;
	Vector3			position;
	Vector2			size;
	float			rotation;
	SRGBA			color;
	
	bool			additive;
	bool			depthRead;
	bool			depthWrite;

	CTexture		*pTexture;
	CObject3D		*pParent;

	Vector2			texcoord[2];
	Vector2			alignment;
	
	bool			locked;
	Vector3			lockedAxis;

public:
	float			screenZ;	

public:
	CSprite();
};

//////////////////////////////////////////////////////////////////////////
class CSprite2D
{
public:
	Vector2			position;
	float			rotation;
	SRGBA			color;
	Vector2			scale;
	Vector2			offset;
	Vector2			texCoords[2];
	CTexture		*pTexture;
	int				z;
	Vector2			size;			// if set, used instead of scale
	bool			visible;
	
public:
	CSprite2D()
	{
		visible= true;
		rotation = 0;
		scale = Vector2(1,1);
		pTexture = NULL;
		texCoords[0] = Vector2(0,0);
		texCoords[1] = Vector2(1,1);
		color = WHITE;
		z = 0;
	}	
};

//////////////////////////////////////////////////////////////////////////
// 3D sprite that behaves like a 3d object
//////////////////////////////////////////////////////////////////////////
class CSprite3D : public CObject3D
{
public:
	TYPE("Sprite");
	CSprite3D(CTexture *texture, float width = 1.0f, float height = 1.0f, bool additive = false);
	virtual ~CSprite3D();

public:
	Vector2			alignment;		// default: centered 0,0	
	Vector2			size;
	CTexture		*pTexture;
	bool			locked;			// is sprite orientation locked to an axis?
	Vector3			lockedAxis;		// locked direction
	bool			additive;
	Vector2			texcoord[2];
	bool			depthRead;
	bool			enemy;

	float			screenZ;		// used for sorting
};