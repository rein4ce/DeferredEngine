#include "platform.h"
#include "Sprite.h"
#include "Resources.h"
#include "ConsolePanel.h"

CSprite3D::CSprite3D(CTexture *texture, float width, float height, bool additive) : CObject3D()
{	
	pTexture = texture;
	size = Vector2(width, height);	
	this->additive = additive;
	screenZ = 0;
	depthRead = true;	

	lockedAxis = Vector3(0,1,0);		// defaut up vector
	locked = false;

	texcoord[0] = Vector2(0,0);
	texcoord[1] = Vector2(1,1);
}

CSprite3D::~CSprite3D()
{
	CONSOLE("~CSprite3D()");
}

CSprite::CSprite()
{
	id = -1;
	rotation = 0;
	color = WHITE;
	additive = false;
	pTexture = NULL;
	texcoord[0] = Vector2(0,1);
	texcoord[1] = Vector2(1,0);
	locked = false;
	lockedAxis = Vector3(0,1,0);
	screenZ = 0;
	pParent = NULL;
	depthRead = true;
	depthWrite = true;
	size = Vector2(1,1);
}