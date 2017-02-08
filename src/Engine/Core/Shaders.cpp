#include "platform.h"
#include "Shader.h"
#include "Shaders.h"
#include "Utils.h"
#include "Material.h"
#include <boost/algorithm/string.hpp>
#include "Direct3D.h"

namespace ShaderChunks
{
	string FogParams[] = {
		"#ifdef USE_FOG",
			"float4 fogColor;",
			"#ifdef FOG_EXP2",
				"float fogDensity;",
			"#else",
				"float fogNear;",
				"float fogFar;",
			"#endif",
		"#endif"
	};
	
	string FogPixel[] = {
		"#ifdef USE_FOG",
			"float depth = input.position.z;",
			"#ifdef FOG_EXP2",
				"float log2 = 1.442695;",
				"float fogFactor = exp2(-fogDensity * fogDensity * depth * depth * log2);",
				"fogFactor = 1.0 - clamp(fogFactor, 0.0, 1.0);",
			"#else",
				"float fogFactor = smoothstep(fogNear, fogFar, depth);",
			"#endif",
			"color = float4(input.fogFactor,0,0,1);",			
		"#endif"
	};

	string FogVertex[] = {
		"#ifdef USE_FOG",
			"float4 cameraPosition = mul(input.position, worldMatrix);",
			"cameraPosition = mul(cameraPosition, viewMatrix);",
			"output.fogFactor = saturate((fogFar - cameraPosition.z) / (fogFar - fogNear));",
		"#endif"
	};

	//////////////////////////////////////////////////////////////////////////
	map<string, string> chunks;
	void InitShaderChunks()
	{
		chunks["$FOG"] = MERGE(FogParams);
		chunks["$FOG_PIXEL"] = MERGE(FogPixel);
		chunks["$FOG_VERTEX"] = MERGE(FogVertex);
	}
}

//////////////////////////////////////////////////////////////////////////
map<string, string>			CShaderFactory::shaderFiles;	
map<string, CShader*>			CShaderFactory::shaders;
map<string, SShaderFragment>	CShaderFactory::fragments;

//////////////////////////////////////////////////////////////////////////
// Load shader files from /shaders folder, process them and store the 
// generated code in a map for later access
//////////////////////////////////////////////////////////////////////////
void CShaderFactory::LoadShaders()
{
	ShaderChunks::InitShaderChunks();
	LoadShaderFragments();

	vector<string> files = GetFilesInDirectory("shaders");
	for (int i=0; i<files.size(); i++)
	{
		if (GetFileExtension(files[i]) != "fx") continue;
		string file = ReadFile(string("shaders/" + files[i]));		
		//trace("Processing shader '"+files[i]+"'");
		ProcessShader(file);
		shaderFiles[files[i]] = file;
	}
}

//////////////////////////////////////////////////////////////////////////
// Load shader fragments
//////////////////////////////////////////////////////////////////////////
void CShaderFactory::LoadShaderFragments()
{
	trace("Loading shader fragments...");
	fragments.clear();

	vector<string> files = GetFilesInDirectory("shaders");
	for (int i=0; i<files.size(); i++)
	{
		if (GetFileExtension(files[i]) != "fragment") continue;
		string file = ReadFile(string("shaders/" + files[i]));

		vector<string> lines;
		boost::split(lines, file, boost::is_any_of("\n"));

		SShaderFragment frag;

		string *part = NULL;
		for (vector<string>::iterator line = lines.begin(); line != lines.end(); line++)
		{
			if (StartsWith(*line, "<Params>"))	part = &frag.params; else
			if (StartsWith(*line, "<Pixel>"))	part = &frag.pixel; else
			if (StartsWith(*line, "<Vertex>")) part = &frag.vertex; else
			if (StartsWith(*line, "</Params>") || StartsWith(*line, "</Pixel>") || StartsWith(*line, "</Vertex>")) part = NULL; else

			if (part != NULL)  *part += *line + "\n";			
		}

		// assign file to the capitalized filename token e.g. FOG
		string name = GetFileName(files[i]);
		boost::to_upper(name);

		fragments["$"+name] = frag;
	}
}

//////////////////////////////////////////////////////////////////////////
// Replace all $ directives with chunks of code 
//////////////////////////////////////////////////////////////////////////
void CShaderFactory::ProcessShader(string &code)
{
	if (code.find("//DONT_PROCESS") != -1) return;

	string vertexShader = "PixelInputType VertexShaderFunc(VertexInputType input)\n{\n\tPixelInputType output;";
	string pixelShader = "float4 PixelShaderFunc(PixelInputType input) : SV_TARGET\n{\n\tfloat4 color = float4(0,0,0,1);\n";
	string shaderBody = \
		"\n\ntechnique10 Main {\n"
		"pass pass0 {\n"
		"SetVertexShader(CompileShader(vs_4_0, VertexShaderFunc()));\n"
		"SetPixelShader(CompileShader(ps_4_0, PixelShaderFunc()));\n"
		"SetGeometryShader(NULL);\n"
		"}}\n";

	// insert vertex and pixel shader function stubs
	code = replace(code, "$VERTEX\n{", vertexShader);
	code = replace(code, "$PIXEL\n{", pixelShader);

	// insert shader fragments
	std::map<string, SShaderFragment>::iterator iter = fragments.begin();
	while (iter != fragments.end())
	{
		SShaderFragment &frag = iter->second;
		code = replace(code, iter->first+"_VERTEX", frag.vertex);		// e.g. $FOG_VERTEX
		code = replace(code, iter->first+"_PIXEL", frag.pixel);			// e.g. $FOG_PIXEL
		code = replace(code, iter->first, frag.params);					// e.g. $FOG
		
		iter++;
	}

	// insert shader body at the end
	code.append(shaderBody);	
}

//////////////////////////////////////////////////////////////////////////
// Create an instance of shader class based on it's filename, generate
// the shader code using the parameters and compile it
//////////////////////////////////////////////////////////////////////////
CShader* CShaderFactory::CreateShader(string name, CMaterial *material, SShaderParameters &params)
{
	CShader *shader = NULL;
	string code;

	// first calculate number of lights in the scene
	int numDirLights = 0;
	int numPointLights = 0;	
	int numSpotLights = 0;

	int numLights = params.lights->Size();		// number of different lights currently in the scene

	// Check if there already has been created the same shader with exact the same params
	stringstream ss;
	//ss << name << material->features << "_" << numDirLights << "_" << numPointLights;	
	ss << name << material->features << "_" << numLights;
	string key = ss.str();

	if (shaders.find(key) != shaders.end())
		return shaders[key];						// identical shader already exists, return it

	// Make sure the shader has been loaded and obtain it's code
	Assert(shaderFiles.find(name) != shaderFiles.end())
	code = shaderFiles[name];
	
	// Create an instance of appropriate shader wrapper class
	shader = new CShader(name);

	// Insert common uniforms
	ss.clear();
	ss.str("");

	ss << "matrix worldMatrix;\n";
	ss << "matrix viewMatrix;\n";
	ss << "matrix projectionMatrix;\n";
	ss << "float3 eye;\n";
	ss << "float realtime;\n";
	
	code = ss.str() + code;
	
	
	//////////////////////////////////////////////////////////////////////////
	// Add shader definitions according to material features
	//////////////////////////////////////////////////////////////////////////

	shader->numPointLights = numPointLights;
	shader->numDirLights = numDirLights;	

	ss.clear();
	ss.str("");

	if (material->features & EShaderFeature::FOG)		ss << "#define USE_FOG\n";
	if (material->features & EShaderFeature::ENVMAP)	ss << "#define USE_ENVMAP\n";
	if (material->features & EShaderFeature::COLOR)		ss << "#define USE_COLOR\n";		
	if (material->features & EShaderFeature::TEXTURE)	ss << "#define USE_TEXTURE\n";
	if (material->features & EShaderFeature::SHADOW)	ss << "#define USE_SHADOW\n";
	if (material->features & EShaderFeature::LIGHT)
	{
		ss << "#define USE_LIGHTS" << "\n";
		ss << "#define MAX_LIGHTS " << numLights << "\n";
	}

	code = ss.str() + code;


	//////////////////////////////////////////////////////////////////////////
	// Add shader vertex attributes
	//////////////////////////////////////////////////////////////////////////
	shader->attributes = material->GetAttributes();

	string definitions;
	for (int i=0; i<2; i++)
	{
		definitions += "struct " + string(i==0 ? "VertexInputType" : "PixelInputType") + "\n{\n";
		if (shader->attributes & EShaderAttribute::POSITION)	definitions += i ? "\tfloat4 position: SV_POSITION;\n" : "\tfloat4 position: POSITION;\n";
		if (shader->attributes & EShaderAttribute::NORMAL)		definitions += "\tfloat3 normal: NORMAL;\n";
		if (shader->attributes & EShaderAttribute::COLOR)		definitions += "\tfloat4 color: COLOR;\n";
		if (shader->attributes & EShaderAttribute::TEXCOORD)	definitions += "\tfloat2 texcoord: TEXCOORD;\n";
		if (shader->attributes & EShaderAttribute::TEXCOORD2)	definitions += "\tfloat2 texcoord2: TEXCOORD1;\n";
		if (shader->attributes & EShaderAttribute::TANGENT)		definitions += "\tfloat3 tangent: TANGENT;\n";			
		
		// Pixel shader struct only
		if (i==1)
		{
			if (material->features & EShaderFeature::FOG)		definitions += "\tfloat fogFactor : FOG;\n";						
			if (material->features & EShaderFeature::LIGHT)		
			{
				definitions += "\tfloat4 lpos: TEXCOORD4;\n";
				definitions += "\tfloat3 binormal: BINORMAL;\n";
			}
			if (material->features & EShaderFeature::ENVMAP)	definitions += "\tfloat3 reflection: TEXCOORD5;\n";
			definitions += "\tfloat4 worldPosition : TEXCOORD2;\n";
			definitions += "\tfloat4 viewDirection : TEXCOORD3;\n";			
		}
		definitions += "};\n\n";
	}	
	code = definitions + code;
	
	// do some post processing
	shader->PostProcessCode(code);
	
	// Add this shader into the hashmap
	shaders[key] = shader;

	WriteFile("generated_" + name, code);

	// Initialize shader variables
	if (shader) shader->Create(code, gD3D->GetDevice());

	return shader;
}

//////////////////////////////////////////////////////////////////////////
// Initialize material's shader if hasn't done yet so
// This will generate new shader based on material's parameters
//////////////////////////////////////////////////////////////////////////
void CShaderFactory::InitMaterial( CMaterial *material, SShaderParameters &params )
{
	// create or reuse shader instance
	material->pShader = CreateShader(material->GetShaderName(), material, params);
}

//////////////////////////////////////////////////////////////////////////
string CShaderFactory::GetShaderCode( string name )
{
	return shaderFiles[name];
}

//////////////////////////////////////////////////////////////////////////
// Get shader for given filename, load if necessary
//////////////////////////////////////////////////////////////////////////
CShader* CShaderFactory::GetShader( string name, int attributes/* = 0*/ )
{
	CShader *shader = shaders[name];
	if (shader != NULL) return shader;
	
	// Make sure the shader has been loaded and obtain it's code
	if (shaderFiles.find(name) == shaderFiles.end())
	{
		Error("Requested shader file %s doesn't exist", name.c_str());
		return NULL;
	}
	string code = shaderFiles[name];

	// Create an instance of appropriate shader wrapper class
	shader = new CShader(name);
	if (attributes != 0) shader->attributes = attributes;
	shader->Create(code, gD3D->GetDevice());

	// Add this shader to the map
	shaders[name] = shader;

	return shader;
}

//////////////////////////////////////////////////////////////////////////
// Release shaders memory
//////////////////////////////////////////////////////////////////////////
void CShaderFactory::Release()
{
	for (map<string, CShader*>::iterator i = shaders.begin(); i != shaders.end(); i++)
	{
		SAFE_DELETE((*i).second);
	}
}