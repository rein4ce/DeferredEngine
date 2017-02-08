#include "platform.h"
#include "Material.h"

//////////////////////////////////////////////////////////////////////////
// Get shader attributes based on features
//////////////////////////////////////////////////////////////////////////
int CMaterial::GetAttributes()
{
	int attr = EShaderAttribute::POSITION;

	if (features & EShaderFeature::COLOR)		attr |= EShaderAttribute::COLOR;
	if (features & EShaderFeature::TEXTURE)		attr |= EShaderAttribute::TEXCOORD;
	if (features & EShaderFeature::LIGHT)
	{
		attr |= EShaderAttribute::NORMAL;
		attr |= EShaderAttribute::TANGENT;
	}

	return attr;
}

//////////////////////////////////////////////////////////////////////////
// Update material flags regarding used textures
//////////////////////////////////////////////////////////////////////////
void CMaterial::RefreshFlags()
{
	SetFlag(flags, MF_DIFFUSE_TEXTURE,		pTexture != NULL);			// diffuse texture
	SetFlag(flags, MF_SPECULAR_TEXTURE,		pSpecularTexture != NULL);	// specular
	SetFlag(flags, MF_NORMAL_TEXTURE,		pNormalTexture != NULL);
	SetFlag(flags, MF_EMMISIVE_TEXTURE,		pEmmisiveTexture != NULL);

	SetFlag(flags, MF_SPECULAR_FROM_ALPHA,	useAlphaSpecular);
	SetFlag(flags, MF_FILTER_NONE,			!useFiltering);
	SetFlag(flags, MF_TEXTURE_CLAMP,		clamp);
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::Create()
{
	static int nextId = 0;
	id = nextId++;
	features = EShaderFeature::LIGHT | EShaderFeature::TEXTURE | EShaderFeature::SHADOW;

	flags = 0;
	opacity = 1.0f;
	transparent = false;
	blending = EBlending::Normal;
	depthTest = true;
	depthWrite = true;
	polygonOffset = false;
	polygonOffsetFactor = 0;
	polygonOffsetUnits = 0;
	alphaTest = 0;
	overdraw = false;
	color = SRGBA(255,255,255,255);
	doubleSided = false;

	envReflectivity = 0.0f;
	reflectivity = 1.0f;
	filename = "empty.fx";
	wireframe = false;

	specular = 1.0f;
	specularPower = 1.0f;

	pShader = NULL;
	clamp = false;

	useFiltering = true;
	useAlphaSpecular = false;
	receiveShadow = true;

	pTexture =
		pSpecularTexture =
		pNormalTexture =
		pEmmisiveTexture = NULL;
	pEnvTexture = NULL;
}

//////////////////////////////////////////////////////////////////////////
CTextureMaterial::CTextureMaterial(string filename)
{
	features = EShaderFeature::TEXTURE;
	pTexture = new CTexture(filename);
}

//////////////////////////////////////////////////////////////////////////
CSpriteMaterial::CSpriteMaterial()
{
	features = EShaderFeature::TEXTURE | EShaderFeature::FOG;
	pTexture = NULL;
	filename = "sprite.fx";
	transparent = true;
}