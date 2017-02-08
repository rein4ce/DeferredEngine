#include "platform.h"
#include "Shader.h"
#include "Light.h"
#include "Material.h"
#include "Object3D.h"
#include "Camera.h"
#include "Scene.h"
#include <sstream>
#include "Engine.h"
#include "Renderer.h"
#include "Game.h"
#include "Geometry.h"
#include "CVar.h"

extern CVar cv_shadowNear;
extern CVar cv_shadowFar;

//////////////////////////////////////////////////////////////////////////
CShader::CShader(string filename)
{
	pEffect = NULL;
	pTechnique = NULL;
	pLayout = NULL;	
	pAttributes = NULL;	
	attributes = EShaderAttribute::POSITION;
	this->filename = filename;

	numDirLights = 0;
	numPointLights = 0;	
}

CShader::~CShader()
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
void CShader::Release()
{
	SAFE_RELEASE(pLayout);
	SAFE_RELEASE(pEffect);
	SAFE_DELETE_ARRAY(pAttributes);
}

//////////////////////////////////////////////////////////////////////////
// Create shader from code, initialize layout and variables
//////////////////////////////////////////////////////////////////////////
bool CShader::Create(string &code, ID3D10Device *pDevice)
{
	ID3D10Blob *error, *shaderBlob;
	const char *filename = this->filename.c_str();
	this->pDevice = pDevice;


	HRESULT hr;
	

	// Create effect from shader code
	if (FAILED(D3DX10CreateEffectFromMemory(code.c_str(), code.length(), filename, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, pDevice, NULL, NULL, &pEffect, &error, NULL )))
	{		
		if (error)
		{
			OutputShaderErrorMessage(error, filename);
		}
		else
		{
			char msg[128];
			sprintf(msg,"Unable to load shader file %s", filename);
			Error(msg);
		}
		return false;
	}
	
	// Process shader and retrieve all variables and map them
	D3D10_EFFECT_DESC desc;
	pEffect->GetDesc(&desc);

	D3D10_EFFECT_TYPE_DESC typeDesc;
	D3D10_EFFECT_VARIABLE_DESC varDesc;
	
	for (int i=0; i<desc.GlobalVariables; i++)
	{
		ID3D10EffectVariable *var = pEffect->GetVariableByIndex(i);
		ID3D10EffectType *varType = var->GetType();
		
		varType->GetDesc(&typeDesc);
		var->GetDesc(&varDesc);

		string name = varDesc.Name;
		EShaderVariableType::Type type = EShaderVariableType::Integer;

		if (!strcmp(typeDesc.TypeName, "int") ||  
			!strcmp(typeDesc.TypeName, "uint"))
			type = EShaderVariableType::Integer;
		else if (!strcmp(typeDesc.TypeName, "float"))  
			type = EShaderVariableType::Float;
		else if (!strcmp(typeDesc.TypeName, "bool"))  
			type = EShaderVariableType::Boolean;
		else if (!strcmp(typeDesc.TypeName, "float2"))  
			type = EShaderVariableType::Vector2;
		else if (!strcmp(typeDesc.TypeName, "float3"))  
			type = EShaderVariableType::Vector3;
		else if (!strcmp(typeDesc.TypeName, "float4"))  
			type = EShaderVariableType::Vector4;
		else if (!strcmp(typeDesc.TypeName, "float4x4"))  
			type = EShaderVariableType::Matrix4;
		else if (!strcmp(typeDesc.TypeName, "Texture2D"))  
			type = EShaderVariableType::Texture;
		else if (!strcmp(typeDesc.TypeName, "TextureCube"))  
			type = EShaderVariableType::CubeTexture;
		else 
			type = EShaderVariableType::Void;
		
		//trace("Mapping variable %s", name.c_str());
		uniforms[name] = SShaderVariable(pEffect, name, type);
	}		
	
	pTechnique = pEffect->GetTechniqueByIndex(0);
	if (!pTechnique) 
	{
		trace("Cannot obtain shader technique pointer: "+this->filename);
		return false;
	}

	// create layout from attributes
	int count = 0;
	uint32 attr = attributes;

	for (int i=0; i<16; i++) 
		if (attr & (1<<i)) count++;

	// static array of available attributes
	static D3D10_INPUT_ELEMENT_DESC Attributes[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD1",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D10_APPEND_ALIGNED_ELEMENT,	D3D10_INPUT_PER_VERTEX_DATA, 0 }		
	};

	// copy the required attributes
	pAttributes = new D3D10_INPUT_ELEMENT_DESC[count];
	for (int i=0, j=0; i<7; i++)		
		if (attr & (1<<i)) memcpy(&pAttributes[j++], &Attributes[i], sizeof(D3D10_INPUT_ELEMENT_DESC));

	SetLayout(pAttributes, count);

	InitVariables();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CShader::InitVariables()
{
	realtime.Init(pEffect, "realtime");		
	diffuseColor.Init(pEffect, "mColor");

	worldMatrix.Init(pEffect, "worldMatrix");
	viewMatrix.Init(pEffect, "viewMatrix");
	projectionMatrix.Init(pEffect, "projectionMatrix");
	eye.Init(pEffect, "eye");

	mDiffuseTexture.Init(pEffect, "mDiffuseTexture");
	mSpecularTexture.Init(pEffect, "mSpecularTexture");
	mNormalTexture.Init(pEffect, "mNormalTexture");
	mEmmisiveTexture.Init(pEffect, "mEmmisiveTexture");

	mColor.Init(pEffect, "mColor");
	mFlags.Init(pEffect, "mFlags");
	mSpecularPower.Init(pEffect, "mSpecularPower");
	mSpecular.Init(pEffect, "mSpecular");
	mReflectivity.Init(pEffect, "mReflectivity");
	mShadow.Init(pEffect, "mShadow");

	ambientColor.Init(pEffect, "ambientColor");
	fogColor.Init(pEffect, "fogColor");
	fogNear.Init(pEffect, "fogNear");
	fogFar.Init(pEffect, "fogFar");

	position.Init(pEffect, "position");
	rotation.Init(pEffect, "rotation");
	alignment.Init(pEffect, "alignment");
	size.Init(pEffect, "size");
	screenPosition.Init(pEffect, "screenPosition");
	locked.Init(pEffect, "locked");
	size.Init(pEffect, "size");
	lockedAxis.Init(pEffect, "lockedAxis");
	diffuse.Init(pEffect, "diffuse");
	texCoords.Init(pEffect, "texCoords");

	lightPosition.Init(pEffect, "lightPosition");
	lightColor.Init(pEffect, "lightColor");
	lightRadius.Init(pEffect, "lightRadius");
	lightIntensity.Init(pEffect, "lightIntensity");
	lightOverbright.Init(pEffect, "lightOverbright");

}

//////////////////////////////////////////////////////////////////////////
// Apply vertex attributes to all passes
//////////////////////////////////////////////////////////////////////////
void CShader::SetLayout( D3D10_INPUT_ELEMENT_DESC *layout, int numElements )
{
	HRESULT result;
	D3D10_TECHNIQUE_DESC techniqueDesc;
	pDevice->IASetInputLayout(pLayout);
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		D3D10_PASS_DESC passDesc;
		pTechnique->GetPassByIndex(i)->GetDesc(&passDesc);
		if ( FAILED(result = pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pLayout ))) 
		{
			Error("Error intializing shader input layout for pass %d: %s",i, this->filename.c_str());
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Show shader error message
//////////////////////////////////////////////////////////////////////////
void CShader::OutputShaderErrorMessage(ID3D10Blob *error, const char *filename)
{
	char *compileErrors;
	uint32 bufferSize;

	compileErrors = (char*)error->GetBufferPointer();
	bufferSize = error->GetBufferSize();

	string fout;
	fout = string("Error compiling shader ") + filename + "\n\n";
	for (int i=0; i<bufferSize; i++)
		fout += compileErrors[i];

	SAFE_RELEASE(error);
	Error(fout.c_str());
}

//////////////////////////////////////////////////////////////////////////
// Set unofrms per material change
//////////////////////////////////////////////////////////////////////////
void CShader::SetMaterialUniforms(CMaterial *material)
{
	// refresh material flags
	material->RefreshFlags();
	
	// texture
	if (material->features & EShaderFeature::TEXTURE)
	{
		if (material->pTexture)	
			mDiffuseTexture.Set(material->pTexture);			

		if (material->pSpecularTexture)	
			mSpecularTexture.Set(material->pSpecularTexture);			

		if (material->pNormalTexture)	
			mNormalTexture.Set(material->pNormalTexture);			

		if (material->pEmmisiveTexture)	
			mEmmisiveTexture.Set(material->pEmmisiveTexture);			
	}

	// material parameters
	mFlags.Set(material->flags);
	mSpecularPower.Set((float)material->specularPower);
	mSpecular.Set((float)material->specular);
	mReflectivity.Set(material->reflectivity);
	mShadow.Set(material->receiveShadow);
}

//////////////////////////////////////////////////////////////////////////
// Set global uniforms for entire frame
//////////////////////////////////////////////////////////////////////////
void CShader::SetGlobalUniforms(CCamera *camera, SShaderParameters *params)
{
	string type;

	realtime.Set(gEngine.realtime);

	ambientColor.Set(*params->ambientColor);
		
	// fog
	if (params->fog)
	{
		fogColor.Set(params->fog->color);
		fogFar.Set(params->fog->far);
		fogNear.Set(params->fog->near);
	}
	

	// camera
	if (camera)
	{
		projectionMatrix.Set(camera->matrixProjection);
		viewMatrix.Set(camera->matrixView);
		eye.Set(camera->matrixWorld.GetPosition());

		int size[2] = { gRenderer.shadowMapViewport.Width, gRenderer.shadowMapViewport.Height };
		uniforms["shadowMapSize"].Set(size, sizeof(int)*2);
	}	
}

//////////////////////////////////////////////////////////////////////////
// Set uniforms for every individual object being drawn
//////////////////////////////////////////////////////////////////////////
void CShader::SetObjectUniforms(CObject3D *object, CMaterial *material)
{
	if (!object) return;

	// mesh
	worldMatrix.Set(object->matrixWorld);

	if (material)
	{
		// object color + opacity
		Vector3 c = material->color;
		Vector3 o = object->color;
		c = Vector3(c.x * o.x, c.y * o.y, c.z * o.z);

		SRGBA color = SRGBA(c.x * 255, c.y*255, c.z*255, 255);
		color.a = material->opacity * 255.0f * object->opacity;

		mColor.Set(color);
	}
	
}

//////////////////////////////////////////////////////////////////////////
void CShader::DrawGroup( CGeometryGroup *group )
{
	uint32 stride = group->vertexSize * sizeof(float);
	uint32 offset = 0;

	pDevice->IASetVertexBuffers(0, 1, &group->pVertexBuffer, &stride, &offset);	
	pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D10_TECHNIQUE_DESC techniqueDesc;
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		pTechnique->GetPassByIndex(i)->Apply(0);
		pDevice->Draw(group->vertexCount, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CShader::Draw(int vertexCount)
{
	D3D10_TECHNIQUE_DESC techniqueDesc;

	pDevice->IASetInputLayout(pLayout);
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		pTechnique->GetPassByIndex(i)->Apply(0);
		pDevice->Draw(vertexCount, 0);
	}
}






















CShaderOld::CShaderOld(void)
{
	pDevice = NULL;
	pEffect = NULL;
	pTechnique = NULL;
	pLayout = NULL;	
	pPass = NULL;
}

CShaderOld::~CShaderOld(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CShaderOld::Release()
{
	pTechnique = NULL;
	pPass = NULL;
	SAFE_RELEASE(pLayout);
	SAFE_RELEASE(pEffect);	
}

//////////////////////////////////////////////////////////////////////////
bool CShaderOld::LoadShader( const char *name, const char *filename, ID3D10Device *pDevice )
{
	HRESULT result;
	ID3D10Blob *error;

	this->name = name;
	this->filename = filename;
	this->pDevice = pDevice;

	// load shader from the file
	if ( FAILED(result = D3DX10CreateEffectFromFile(filename, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, pDevice, NULL, NULL, &pEffect, &error, NULL ))) 
	{
		if (error)
		{
			OutputShaderErrorMessage(error, filename);
		}
		else
		{
			char msg[128];
			sprintf(msg,"Unable to load shader file %s", filename);
			MessageBox(NULL, filename, msg, MB_OK);
		}
		return false;
	}

	pTechnique = pEffect->GetTechniqueByIndex(0);
	if (!pTechnique) 
	{
		trace("Cannot obtain shader technique pointer");
		return false;
	}

	pPass = pTechnique->GetPassByIndex(0);
	if (!pPass)
	{
		trace("Cannot obtain shader technique pass pointer");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CShaderOld::SetLayout( D3D10_INPUT_ELEMENT_DESC *layout, int numElements )
{
	HRESULT result;
	D3D10_PASS_DESC passDesc;
	pPass->GetDesc(&passDesc);

	if ( FAILED(result = pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pLayout ))) 
	{
		trace("Error intializing shader input layout");
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CShaderOld::OutputShaderErrorMessage(ID3D10Blob *error, const char *filename)
{
	char *compileErrors;
	uint32 bufferSize;

	compileErrors = (char*)error->GetBufferPointer();
	bufferSize = error->GetBufferSize();

	string fout;
	fout = string("Error compiling shader ") + filename + "\n\n";
	for (int i=0; i<bufferSize; i++)
		fout += compileErrors[i];


	SAFE_RELEASE(error);
	MessageBox(NULL, fout.c_str(), "Error", MB_OK);
}

//////////////////////////////////////////////////////////////////////////
void CShaderOld::Render(int indexCount)
{
	D3D10_TECHNIQUE_DESC techniqueDesc;

	pDevice->IASetInputLayout(pLayout);
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		pTechnique->GetPassByIndex(i)->Apply(0);
		pDevice->Draw(indexCount, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CShaderOld::RenderMesh(ID3DX10Mesh *mesh)
{
	D3D10_TECHNIQUE_DESC techniqueDesc;

	pDevice->IASetInputLayout(pLayout);
	pTechnique->GetDesc(&techniqueDesc);

	for (int i = 0; i < techniqueDesc.Passes; i++)
	{
		pTechnique->GetPassByIndex(i)->Apply(0);
		mesh->DrawSubset(0);
	}
}




