#pragma once
#include "platform.h"
#include "Utils.h"
#include "Shaders.h"
#include "Utils3D.h"
#include "Array.h"

class CTexture;

// Material flags
#define MF_DIFFUSE_TEXTURE			(1<<0)
#define MF_SPECULAR_TEXTURE			(1<<1)
#define MF_NORMAL_TEXTURE			(1<<2)
#define MF_EMMISIVE_TEXTURE			(1<<3)
#define MF_SPECULAR_FROM_ALPHA		(1<<4)		// use diffuse map's alpha channel for specular
#define MF_FILTER_NONE				(1<<5)		// pixel sampler (no filtering)
#define MF_TEXTURE_CLAMP			(1<<6)		// clamp to edge

class CMaterial
{
public:
	TYPE("Material");
	CMaterial()
	{
		Create();
	}

	CMaterial(string filename, SRGBA color = WHITE)
	{
		Create();
		this->filename = filename;
		this->color = color;
		this->pTexture = CTexture::Get(filename);
	}

	CMaterial(SRGBA color)
	{
		Create();
		this->color = color;
	}

	void	Create();
	void	RefreshFlags();	
	int		GetAttributes();

	virtual inline string GetShaderName() 
	{ 
		return filename;		
	};

public:	
	int				id;					// unique instance id
	uint32			features;			// shader flags 
	string			filename;
	int				flags;
	
	CShader*		pShader;			// pointer to compiled shader class
	
	float			opacity;
	bool			transparent;
	EBlending::Type	blending;
	float			specular;			// specular intensity
	float			specularPower;		// higher value - smaller the highlight
	bool			receiveShadow;

	SRGBA			color;

	CTexture		*pTexture;
	CTexture		*pSpecularTexture;
	CTexture		*pNormalTexture;
	CTexture		*pEmmisiveTexture;	
	CCubeTexture	*pEnvTexture;

	float			envReflectivity;

	float			reflectivity;
	
	bool			useAlphaSpecular;	// alpha channel of diffuse texture used as specular
	bool			useFiltering;		// is texture filtered

	bool			depthTest;
	bool			depthWrite;
	bool			clamp;
	
	bool			polygonOffset;
	float			polygonOffsetFactor;
	float			polygonOffsetUnits;
	float			alphaTest;
	bool			overdraw;
	bool			wireframe;
	bool			doubleSided;
};

//////////////////////////////////////////////////////////////////////////
class CLineBasicMaterial : public CMaterial
{
public:
	TYPE("LineBasicMaterial")
	CLineBasicMaterial()
	{
		color = SRGBA(255,255,255);
		lineWidth = 1.0f;
		vertexColors = false;		
	}

public:	
	SRGBA			color;
	float			lineWidth;
	bool			vertexColors;	
};

//////////////////////////////////////////////////////////////////////////
class CDepthMaterial : public CMaterial
{
public:
	TYPE("DepthMaterial")
	CDepthMaterial()
	{
		filename = "shadowmap.fx";
		features = 0;
	}

	virtual inline string GetShaderName() { return "shadowmap.fx"; };
};

//////////////////////////////////////////////////////////////////////////
class CParticleBasicMaterial : public CMaterial
{
public:
	TYPE("ParticleBasicMaterial")
	CParticleBasicMaterial()
	{
		color = SRGBA(255,255,255);
		texture = NULL;
		size = 1.0f;
		sizeAttenuation = true;
		vertexColors = false;
		fog = true;
	}

public:
	SRGBA			color;
	CTexture		*texture;
	float			size;
	bool			sizeAttenuation;
	bool			vertexColors;
	bool			fog;
};

//////////////////////////////////////////////////////////////////////////
class CShaderMaterial : public CMaterial
{
public:
	TYPE("ShaderMaterial")
	CShaderMaterial()
	{
		fragmentShader = "void main() {}";
		vertexShader = "void main() {}";		
		shading = EShading::Smooth;
		wireframe = false;
		wireframeLineWidth = 1.0f;
		fog = false;
		lights = false;
		vertexColors = false;
		skinning = false;
		morphTargets = false;		
	}

public:
	string			fragmentShader;
	string			vertexShader;
	CArray<void*>	uniforms;
	CArray<void*>	attributes;

	EShading::Type	shading;
	bool			wireframe;
	bool			wireframeLineWidth;
	bool			fog;
	bool			lights;
	bool			vertexColors;
	bool			skinning;
	bool			morphTargets;
};

//////////////////////////////////////////////////////////////////////////
class CTextureMaterial : public CMaterial
{
public:
	TYPE("TextureMaterial");
	CTextureMaterial(string filename);
};

//////////////////////////////////////////////////////////////////////////
class CSpriteMaterial : public CMaterial
{
public:
	TYPE("TextureMaterial");
	CSpriteMaterial();
};