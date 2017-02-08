#pragma once
#include "platform.h"
#include "Vector.h"
#include "Matrix.h"
#include "Face.h"
#include "Array.h"
#include "Utils3D.h"

struct SBBox;
class CMaterial;
class CGeometry;
class CRenderer;


struct SMorphTarget
{
	string				name;
	CArray<Vector3>		vertices;
	CArray<SRGBA>		colors;
};

//////////////////////////////////////////////////////////////////////////
// Geometry group is a batch of triangles with a _single_ material
// and single global state to be rendered by renderer
// It has it's own DX buffers, however the actual geometry is kept
// in parent Geometry object
//
// Geometry Group is constructed once when the object is being added to the scene
// Geometry is sliced up int oseparate groups by the material and max limit of
// vertices
//////////////////////////////////////////////////////////////////////////
class CGeometryGroup
{
	friend class CRenderer;
	friend class CGeometry;
	friend class CShader;

private:
	CGeometryGroup() {}
public:
	CGeometryGroup(CGeometry *parent);
	~CGeometryGroup();

	void				Initialize();	

	CArray<Face3>		faces;	
	int					materialIndex;			// index of material in parent geometry object

protected:
	void				CreateVertexArray();
	void				CreateBuffers();
	void				RefreshBuffers();
	
protected:
	CGeometry			*pParent;				// pointer to parent geometry object

	float				*pVertexArray;			
	
	int					vertexCount;
	int					vertexSize;
	int					faceCount;
	int					lineCount;
	int					bufferSize;	

protected:
	ID3D10Buffer		*pVertexBuffer;			// internal DX buffers for sending to GPU
	
	ID3D10Buffer		*pFaceBuffer;
	ID3D10Buffer		*pLineBuffer;
};

//////////////////////////////////////////////////////////////////////////
class CGeometry
{
	friend class CRenderer;
	friend class CScene;
public:
	CGeometry(string filename = "");
	virtual ~CGeometry();

	CGeometry*	Copy();
	bool		Load(string filename);
	void		Reset();

	void		ApplyMatrix(Matrix4 matrix);
	void		ComputeCentroids();
	void		ComputeFaceNormals();
	void		ComputeVertexNormals();
	void		ComputeTangents();	
	void		ComputeBoundingShape();
	void		MergeVertices();
	bool		IsDirty();
	void		Offset(Vector3 offset);
	
	CMaterial*	GetMaterial(int index);

public:
	void		Initialize();					// initialize for rendering
	void		CreateGroupsByMaterial();	

protected:
	void		ComputeBoundingBox();
	void		ComputeBoudingSphere();
	
public:
	static uint32		nextId;

	int					id;						// unique id of this geometry instance

	CArray<Vector3>		vertices;		
	CArray<SRGBA>		colors;
	CArray<CMaterial*>	materials;
	CArray<Face3>		faces;	
	
	CArray<CGeometryGroup*>		groups;						// geometry groups for this object

	SBBox				boundingBox;
	float				boundingSphere;
	bool				dynamic;

	bool				useVertexNormals;					// instead of face normals?

public:
	bool				dirtyVertices;						// which elements has changed (abd thus groups' buffers should be regenerated
	bool				dirtyMorphTargets;
	bool				dirtyElements;
	bool				dirtyUVs;
	bool				dirtyNormals;
	bool				dirtyTangents;
	bool				dirtyColors;

private:
	bool				initialized;
};
