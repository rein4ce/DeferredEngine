#pragma once
#include "Object3d.h"
#include "Array.h"
#include "Utils.h"
#include "Renderer.h"

class CMaterial;
class CGeometry;
class CLight;
struct SRenderObject;
class CCamera;
class CMesh;
class CSprite;
class CSprite3D;
class CObject3D;

struct SFog 
{
	SRGBA		color;
	float		near, far;

	SFog(SRGBA color = WHITE, float near = 1.0f, float far = 5.0f)
	{
		this->color = color;
		this->near = near;
		this->far = far;
	}
};

//////////////////////////////////////////////////////////////////////////
class CScene : public CObject3D
{
	friend class CRenderer;
public:
	CScene(void);
	virtual ~CScene(void);

	TYPE("Scene");

	void			Release();

	void			AddObject(CObject3D *object);
	void			RemoveObject(CObject3D *object);
	void			UpdateObject(CObject3D &object);

	void			AddSprite(CSprite *spr);
	void			RemoveSprite(CSprite *spr);
	void			RemoveSpriteAt(int index);

	void			CreateSkybox(string folderName);

	void			Update(float frametime);

protected:
	void			InitObjects();
	void			AddRenderObject(CObject3D &object);
	void			RemoveRenderObject(CObject3D *object);
	void			UpdateRenderObject(CObject3D &object);
	void			SetupMatrices(CCamera &camera);

public:
	SFog					*fog;
	CMaterial				*overrideMaterial;
	CMesh					*pSkybox;

	CArray<CObject3D*>		objects;
	CArray<CLight*>			lights;
	CArray<CSprite3D*>		sprites3d;	
	CArray<CSprite*>		sprites;		

	SRGBA					ambientColor;

public:
	static CObject3D*		CreateMultiMaterialObject(CGeometry *geometry, CArray<CMaterial*> materials);
	static CObject3D*		CloneObject(CObject3D *source);

protected:
	CArray<CObject3D*>		objectsAdded;			// objects to be added next frame
	CArray<CObject3D*>		objectsRemoved;			// objects to be removed next frame
	CArray<SRenderObject>	renderObjects;			// list of objects to be rendered
};
