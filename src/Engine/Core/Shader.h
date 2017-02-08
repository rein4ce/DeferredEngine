#pragma once
#include "platform.h"
#include "Vector.h"
#include "Utils.h"
#include "Matrix.h"
#include "Resources.h"
#include "Array.h"
#include <boost/unordered_map.hpp>
#include <D3D10.h>

class CShaderFactory;
class CMaterial;
class CLight;
class CCamera;
class CObject3D;
struct SFog;
class CGeometryGroup;

#define DEBUG_SHADERS		false

//////////////////////////////////////////////////////////////////////////
// Shader vertex attribute flags - they must match the declarations
// in CShader::Init function
//////////////////////////////////////////////////////////////////////////
namespace EShaderAttribute { enum 
{
	POSITION		=	(1<<0),
	NORMAL			=	(1<<1),
	COLOR			=	(1<<2),
	TEXCOORD		=	(1<<3),
	TEXCOORD2		=	(1<<4),
	TANGENT			=	(1<<5)
}; };

//////////////////////////////////////////////////////////////////////////
// Shader/material features
//////////////////////////////////////////////////////////////////////////
namespace EShaderFeature { enum 
{
	FOG				=	(1<<0),
	TEXTURE			=	(1<<1),
	ENVMAP			=	(1<<2),
	COLOR			=	(1<<3),
	LIGHT			=	(1<<4),
	SHADOW			=	(1<<5)
}; };

//////////////////////////////////////////////////////////////////////////
// Shader parameters required to build shader code (global state)
//////////////////////////////////////////////////////////////////////////
struct SShaderParameters
{
	SRGBA				*ambientColor;
	SFog				*fog;
	CArray<CLight*>		*lights;
	bool				shadowPass;
	CArray<CCamera*>	shadowCasters;
};

//////////////////////////////////////////////////////////////////////////
// Different types of shader uniforms
//////////////////////////////////////////////////////////////////////////
template<class T>
struct SShaderUniform 
{	
	void Init(ID3D10Effect *pEffect, string name) = 0;
	void Set(T value) = 0;
};


namespace EShaderVariableType { enum Type 
{
	Boolean,
	Float,
	Integer,
	Vector2,
	Vector3,
	Vector4,
	Matrix4,
	Texture,
	CubeTexture,
	Void
}; };


struct SShaderVariable
{
	SShaderVariable(){ initialized = false; }
	SShaderVariable(ID3D10Effect *pEffect, string name, EShaderVariableType::Type type)
	{
		Init(pEffect, name, type);
	}

	void Init(ID3D10Effect *pEffect, string name, EShaderVariableType::Type type)
	{
		switch (type)
		{
		case EShaderVariableType::Boolean:
		case EShaderVariableType::Float:
		case EShaderVariableType::Integer:
			pVariable = pEffect->GetVariableByName(name.c_str())->AsScalar();
			break;

		case EShaderVariableType::Vector2:
		case EShaderVariableType::Vector3:
		case EShaderVariableType::Vector4:
			pVariable = pEffect->GetVariableByName(name.c_str())->AsVector();
			break;

		case EShaderVariableType::Matrix4:
			pVariable = pEffect->GetVariableByName(name.c_str())->AsMatrix();
			break;

		case EShaderVariableType::Texture:
		case EShaderVariableType::CubeTexture:
			pVariable = pEffect->GetVariableByName(name.c_str())->AsShaderResource();
			break;

		case EShaderVariableType::Void:
			pVariable = pEffect->GetVariableByName(name.c_str());
			break;
		}
		this->type = type;
		initialized = true;
	}

	void operator =(bool value) { Set(value); }
	void Set(bool value)
	{
		if (!initialized) return;
		((ID3D10EffectScalarVariable*)pVariable)->SetBool(value);
	}

	void operator =(float value) { Set(value); }
	void Set(float value)
	{
		if (!initialized) return;
		((ID3D10EffectScalarVariable*)pVariable)->SetFloat(value);
	}

	void operator =(double value) { Set(value); }
	void Set(double value)
	{
		if (!initialized) return;
		((ID3D10EffectScalarVariable*)pVariable)->SetFloat((float)value);
	}
	
	void operator =(int value) { Set(value); }
	void Set(int value)
	{
		if (!initialized) return;
		((ID3D10EffectScalarVariable*)pVariable)->SetInt(value);
	}

	void operator =(Vector2 &value) { Set(value); }
	void Set(Vector2 &value)
	{
		if (!initialized) return;
		((ID3D10EffectVectorVariable*)pVariable)->SetFloatVector((float*)&value);
	}
	
	void operator =(Vector3 &value) { Set(value); }
	void Set(Vector3 &value)
	{
		if (!initialized) return;
		((ID3D10EffectVectorVariable*)pVariable)->SetFloatVector((float*)&value);
	}

	void operator =(Vector4 &value) { Set(value); }
	void Set(Vector4 &value)
	{
		if (!initialized) return;
		((ID3D10EffectVectorVariable*)pVariable)->SetFloatVector((float*)&value);
	}

	void operator =(SRGBA &value) { Set(value); }
	void Set(SRGBA &value)
	{
		if (!initialized) return;
		float v[4] = { (float)value.r/255.0f, (float)value.g/255.0f, (float)value.b/255.0f, (float)value.a/255.0f };
		((ID3D10EffectVectorVariable*)pVariable)->SetFloatVector((float*)v);
	}

	void operator =(SRGB &value) { Set(value); }
	void Set(SRGB &value)
	{
		if (!initialized) return;
		float v[4] = { (float)value.r/255.0f, (float)value.g/255.0f, (float)value.b/255.0f, 1.0f };
		((ID3D10EffectVectorVariable*)pVariable)->SetFloatVector((float*)v);
	}

	void operator =(Matrix4 &value) { Set(value); }
	void Set(Matrix4 &value)
	{
		if (!initialized) return;
		((ID3D10EffectMatrixVariable*)pVariable)->SetMatrix((float*)&((D3DXMATRIX)value));
	}

	void operator =(CTexture &value) { Set(value); }
	void Set(CTexture &value)
	{
		if (!initialized) return;
		((ID3D10EffectShaderResourceVariable*)pVariable)->SetResource(value.GetTexture());
	}

	void operator =(ID3D10ShaderResourceView *value) { SetResource(value); }
	void SetResource(ID3D10ShaderResourceView *value)
	{
		if (!initialized) return;
		((ID3D10EffectShaderResourceVariable*)pVariable)->SetResource(value);
	}

	void Set(void *data, int size)
	{
		if (!initialized) return;
		pVariable->SetRawValue(data, 0, size);
	}

	EShaderVariableType::Type		type;
	bool initialized;
	
private:
	ID3D10EffectVariable *pVariable;
	
};


template<>
struct SShaderUniform<bool>
{
	ID3D10EffectScalarVariable *pScalar;
	virtual void Init(ID3D10Effect *pEffect, string name) { pScalar = pEffect->GetVariableByName(name.c_str())->AsScalar(); if(!pScalar->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	virtual void Set(bool value) { pScalar->SetBool(value); }
};

template<>
struct SShaderUniform<int>
{
	ID3D10EffectScalarVariable *pScalar;
	inline void Init(ID3D10Effect *pEffect, string name) { pScalar = pEffect->GetVariableByName(name.c_str())->AsScalar(); if(!pScalar->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(int value) { pScalar->SetInt(value); }
};

template<>
struct SShaderUniform<float>
{
	ID3D10EffectScalarVariable *pScalar;
	inline void Init(ID3D10Effect *pEffect, string name) { pScalar = pEffect->GetVariableByName(name.c_str())->AsScalar(); if(!pScalar->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(float value) { pScalar->SetFloat(value); }
};

template<>
struct SShaderUniform<Vector3>
{
	ID3D10EffectVectorVariable *pVector;
	inline void Init(ID3D10Effect *pEffect, string name) { pVector = pEffect->GetVariableByName(name.c_str())->AsVector(); if(!pVector->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(Vector3& value) 
	{ 
		float v[4] = { value.x, value.y, value.z, 0 };
		pVector->SetFloatVector(v); 
	}
};

template<>
struct SShaderUniform<Vector4>
{
	ID3D10EffectVectorVariable *pVector;
	inline void Init(ID3D10Effect *pEffect, string name) { pVector = pEffect->GetVariableByName(name.c_str())->AsVector(); if(!pVector->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(Vector4& value) { pVector->SetFloatVector((float*)&value); }
};

template<>
struct SShaderUniform<Vector2>
{
	ID3D10EffectVectorVariable *pVector;
	inline void Init(ID3D10Effect *pEffect, string name) { pVector = pEffect->GetVariableByName(name.c_str())->AsVector(); if(!pVector->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(Vector2& value) { pVector->SetFloatVector((float*)&value); }
};

template<>
struct SShaderUniform<SRGBA>
{
	ID3D10EffectVectorVariable *pVector;
	inline void Init(ID3D10Effect *pEffect, string name) { pVector = pEffect->GetVariableByName(name.c_str())->AsVector(); if(!pVector->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(SRGBA& value) 
	{ 
		float v[4] = { (float)value.r/255.0f, (float)value.g/255.0f, (float)value.b/255.0f, (float)value.a/255.0f };
		pVector->SetFloatVector((float*)v); 
	}
};

template<>
struct SShaderUniform<SRGB>
{
	ID3D10EffectVectorVariable *pVector;
	inline void Init(ID3D10Effect *pEffect, string name) { pVector = pEffect->GetVariableByName(name.c_str())->AsVector(); if(!pVector->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(SRGB& value) 
	{ 
		float v[3] = { (float)value.r/255.0f, (float)value.g/255.0f, (float)value.b/255.0f };
		pVector->SetFloatVector((float*)v); 
	}
};

template<>
struct SShaderUniform<Matrix4>
{
	ID3D10EffectMatrixVariable *pMatrix;
	inline void Init(ID3D10Effect *pEffect, string name) { pMatrix = pEffect->GetVariableByName(name.c_str())->AsMatrix(); if(!pMatrix->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(Matrix4& value) { pMatrix->SetMatrix((float*)&((D3DXMATRIX)value)); }
};

template<>
struct SShaderUniform<CTexture>
{
	ID3D10EffectShaderResourceVariable *pTexture;
	inline void Init(ID3D10Effect *pEffect, string name) { pTexture = pEffect->GetVariableByName(name.c_str())->AsShaderResource(); if(!pTexture->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(CTexture *value) { pTexture->SetResource(value ? value->GetTexture() : NULL); }
};

template<>
struct SShaderUniform<ID3D10ShaderResourceView>
{
	ID3D10EffectShaderResourceVariable *pTexture;
	inline void Init(ID3D10Effect *pEffect, string name) { pTexture = pEffect->GetVariableByName(name.c_str())->AsShaderResource(); if(!pTexture->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(ID3D10ShaderResourceView *value) { pTexture->SetResource(value); }
};

template<>
struct SShaderUniform<void*>
{
	ID3D10EffectVariable *pValues;
	inline void Init(ID3D10Effect *pEffect, string name) { pValues = pEffect->GetVariableByName(name.c_str()); if(!pValues->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(void *value, int size) { pValues->SetRawValue(value, 0, size); }
};

template<>
struct SShaderUniform<CCubeTexture>
{
	ID3D10EffectShaderResourceVariable *pTexture;
	inline void Init(ID3D10Effect *pEffect, string name) { pTexture = pEffect->GetVariableByName(name.c_str())->AsShaderResource(); if(!pTexture->IsValid()) if (DEBUG_SHADERS) trace("Shader variable '"+name+"' is invalid"); }
	inline void Set(CCubeTexture *value) { pTexture->SetResource(value->pTexture); }
};






//////////////////////////////////////////////////////////////////////////
// Shader wrapper base class
// contains common shader parameters
//////////////////////////////////////////////////////////////////////////
class CShader
{
	friend class CRenderer;
	friend class CShaderFactory;

public:
	CShader(string filename);
	virtual ~CShader();

	bool	Create(string &code, ID3D10Device *pDevice);
	void	InitVariables();
	void	Release();

	virtual void SetGlobalUniforms(CCamera *camera, SShaderParameters *params);
	virtual void SetMaterialUniforms(CMaterial *material);
	virtual void SetObjectUniforms(CObject3D *object, CMaterial *material);
	virtual void PostProcessCode(string &code) {};

	void	DrawGroup(CGeometryGroup *group);
	void	Draw(int vertexCount);

	SShaderVariable& operator[](string name) { return uniforms[name]; }

protected:
	void	SetLayout(D3D10_INPUT_ELEMENT_DESC *layout, int numElements);	
	void	OutputShaderErrorMessage(ID3D10Blob* error, const char* filename);

public:
	string						filename;	
	uint32						attributes;		// attributes given shader uses

	int							numDirLights;
	int							numPointLights;	

	boost::unordered_map<string, SShaderVariable>	uniforms;

	// G-Buffer shader
	SShaderUniform<float>		realtime;	
	SShaderUniform<SRGBA>		diffuseColor;

	SShaderUniform<Matrix4>		worldMatrix;
	SShaderUniform<Matrix4>		viewMatrix;
	SShaderUniform<Matrix4>		projectionMatrix;
	SShaderUniform<Vector3>		eye;

	SShaderUniform<CTexture>	mDiffuseTexture;
	SShaderUniform<CTexture>	mSpecularTexture;
	SShaderUniform<CTexture>	mNormalTexture;
	SShaderUniform<CTexture>	mEmmisiveTexture;

	SShaderUniform<SRGBA>		mColor;
	SShaderUniform<int>			mFlags;
	SShaderUniform<float>		mSpecularPower;
	SShaderUniform<float>		mSpecular;
	SShaderUniform<float>		mReflectivity;
	SShaderUniform<bool>		mShadow;

	SShaderUniform<SRGBA>		ambientColor;
	SShaderUniform<SRGBA>		fogColor;
	SShaderUniform<float>		fogNear;
	SShaderUniform<float>		fogFar;	

	// Sprite shader
	SShaderUniform<Vector3>		position;
	SShaderUniform<float>		rotation;
	SShaderUniform<Vector2>		alignment;
	SShaderUniform<Vector2>		size;
	SShaderUniform<bool>		screenPosition;
	SShaderUniform<bool>		locked;
	SShaderUniform<Vector3>		lockedAxis;
	SShaderUniform<SRGBA>		diffuse;
	SShaderUniform<Vector4>		texCoords;

	// Light shader
	SShaderUniform<Vector3>		lightPosition;
	SShaderUniform<SRGBA>		lightColor;
	SShaderUniform<float>		lightRadius;
	SShaderUniform<float>		lightIntensity;
	SShaderUniform<float>		lightOverbright;

protected:	
	ID3D10Device				*pDevice;
	ID3D10Effect				*pEffect;
	ID3D10EffectTechnique		*pTechnique;
	ID3D10InputLayout			*pLayout;
	D3D10_INPUT_ELEMENT_DESC	*pAttributes;
};


//////////////////////////////////////////////////////////////////////////
// Shader fragment definitions stored in separate code files
//////////////////////////////////////////////////////////////////////////
struct SShaderFragment
{
	string params;
	string pixel;
	string vertex;
};

//////////////////////////////////////////////////////////////////////////
// Shader factory initializes the shader files, maps them out by name
// and allows creation of shaders on request in runtime
//////////////////////////////////////////////////////////////////////////
class CShaderFactory
{
public:	
	static void			LoadShaders();	
	static void			ProcessShader(string &code);
	static void			InitMaterial(CMaterial *material, SShaderParameters &params);
	static string		GetShaderCode(string name);
	static CShader*		GetShader(string name, int attributes = 0);
	static void			Release();

protected:
	static void			LoadShaderFragments();
	static CShader*		CreateShader(string name, CMaterial* material, SShaderParameters &params);

protected:
	static map<string, string>		shaderFiles;	// shader filename -> processed code
	static map<string, CShader*>	shaders;		// filename + flags -> CShader class
	static map<string, SShaderFragment>	fragments;	// capital case name -> fragment struct
};






class CShaderOld
{
public:
	CShaderOld(void);
	virtual ~CShaderOld(void);

	virtual bool	Init(ID3D10Device *pDevice) = 0;
	void			Release();
	void			Render(int indexCount);
	void			RenderMesh(ID3DX10Mesh *mesh);

protected:
	bool	LoadShader(const char *name, const char *filename, ID3D10Device *pDevice);	
	void	SetLayout(D3D10_INPUT_ELEMENT_DESC *layout, int numElements);	

private:
	void	OutputShaderErrorMessage(ID3D10Blob* error, const char* filename);

protected:
	ID3D10Device				*pDevice;
	ID3D10Effect				*pEffect;
	ID3D10EffectTechnique		*pTechnique;
	ID3D10EffectPass			*pPass;
	ID3D10InputLayout			*pLayout;

	std::string					name;
	std::string					filename;
};










