#include "platform.h"
#include "LightEntity.h"
#include "Light.h"
#include "Properties.h"
#include "Geometries\CubeGeometry.h"
#include "Properties.h"

CLightEntity::CLightEntity(void)
{
	this->geometry = new CCubeGeometry(0.5f, 0.5f, 0.5f);
	editorOnly = true;
	this->CalculateBoundingShape();
	pLight = new CPointLight(Vector3());
	color = RED;
	pLight->color = color;
	this->Add(pLight);
	serialized = true;

	properties.Add(new SPropertyDefine<SRGBA>("Color", &color, PROPERTY_STRING));
}

CLightEntity::~CLightEntity(void)
{
}

void CLightEntity::Update( float frametime )
{
	CEntity::Update(frametime);

	pLight->color = color;	
}
