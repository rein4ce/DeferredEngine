#include "platform.h"
#include "Scene.h"
#include "Mesh.h"
#include "Geometries.h"
#include "Renderer.h"
#include "Material.h"

CScene::CScene(void)
{
	fog = NULL;
	matrixAutoUpdate = false;
	overrideMaterial = NULL;
	ambientColor = SRGBA(255,255,255,255);
	pSkybox = NULL;
}

CScene::~CScene(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
void CScene::AddObject( CObject3D *object )
{
	string type = object->GetType();
	if (type == "DirectionalLight" || type == "PointLight" || type == "SpotLight")
	{		
		if (lights.Find((CLight*)object) == -1)
			lights.AddToTail((CLight*)object);
	}
	else if (type == "Sprite")
	{
		if (sprites3d.Find((CSprite3D*)object) == -1)
			sprites3d.AddToTail((CSprite3D*)object);
	} 
	else
	{
		if ( !(object->GetType() == "Camera" || object->GetType() == "Bone") )
		{
			if (objects.Find(object) == -1)
			{
				objects.AddToTail(object);
				objectsAdded.AddToTail(object);

				// check if previously removed
				objectsRemoved.Remove(object);
			}
		}
	}

	// add object's children to the scene as well
	for (int i=0; i<object->children.Size(); i++)
		AddObject(object->children[i]);
}

//////////////////////////////////////////////////////////////////////////
// Mark given object and it's children from deletion.
// They will be removed next frame
//////////////////////////////////////////////////////////////////////////
void CScene::RemoveObject( CObject3D *object )
{
	object->deleteMe = true;
	string type = object->GetType();
	
	if (type == "DirectionalLight" || type == "PointLight" || type == "SpotLight")
	{
		lights.Remove((CLight*)object);			// if light, remove from lights list
	}
	else if (object->GetType() == "Sprite")
	{
		sprites3d.Remove((CSprite3D*)object);	// if sprite, remove from sprites list	
	}
	else
	{
		if (object->GetType() != "Camera")
		{
			int index = objects.Find(object);	// if it's a normal object, find and remove
			if (index != -1)
			{
				objects.RemoveAt(index);			
				objectsRemoved.AddToTail(object);

				// check if previously added
				objectsAdded.Remove(object);
			}
		}
	}

	// remove object's children from the scene as well
	for (int i=0; i<object->children.Size(); i++)
		RemoveObject(object->children[i]);	
}

//////////////////////////////////////////////////////////////////////////
CObject3D* CScene::CreateMultiMaterialObject( CGeometry *geometry, CArray<CMaterial*> materials )
{
	CObject3D *group;
	for (int i=0; i<materials.Size(); i++)
	{
		CMesh *object = new CMesh(geometry, materials[i]);
		group->Add(object);
	}

	return group;
}

//////////////////////////////////////////////////////////////////////////
CObject3D* CScene::CloneObject( CObject3D *source )
{
	Assert(source);
	CObject3D *object = NULL;

	return object;
}

//////////////////////////////////////////////////////////////////////////
// Add newly added objects to the rendering list, same for removed ones,
// update object buffers
//////////////////////////////////////////////////////////////////////////
void CScene::InitObjects()
{
	// add new scene objects to the scene
	while (objectsAdded.Size())
	{
		AddRenderObject( *objectsAdded[0] );
		objectsAdded.RemoveAt(0);
	}
	
	// and remove objects marked for deletion
	for (int i=0; i<objectsRemoved.Size(); i++)
	{		
		CObject3D *o = objectsRemoved[i];
		RemoveRenderObject( o );			// and it's render onject as well		
	}
	objectsRemoved.Purge();
	
	// update all new objects
	for (int i=0; i<renderObjects.Size(); i++)
		UpdateRenderObject( *(renderObjects[i].object) );
}

//////////////////////////////////////////////////////////////////////////
// New object requires initialization of it's geometry and also adding
// the geometry with the instance into the internal rendering list
// that will later be sorted and rendered
//////////////////////////////////////////////////////////////////////////
void CScene::AddRenderObject(CObject3D &object)
{
	UpdateObject(object);
}

//////////////////////////////////////////////////////////////////////////
// Remove given object from the rendering list
//////////////////////////////////////////////////////////////////////////
void CScene::RemoveRenderObject( CObject3D *object )
{
	for (int i=0; i<renderObjects.Size(); i++)
		if (renderObjects[i].object->id == object->id)			// pointers must match
		{
			renderObjects.RemoveAt(i--);			
		}
}

//////////////////////////////////////////////////////////////////////////
// Update buffer contents of render objects
//////////////////////////////////////////////////////////////////////////
void CScene::UpdateRenderObject(CObject3D &object)
{
	if (object.geometry)
	{		
 		bool dirty = object.geometry->IsDirty();

		// if mesh buffer is dirty, resend entire buffer to GPU
		if (dirty) UpdateObject(object);
	}
}

//////////////////////////////////////////////////////////////////////////
// Calculate model view matrices for all objects in the scene
//////////////////////////////////////////////////////////////////////////
void CScene::SetupMatrices( CCamera &camera )
{
	for (int i=0; i<renderObjects.Size(); i++)
	{		
		renderObjects[i].render = false;
		CObject3D *o = renderObjects[i].object;

		if (!o->visible || !o->geometry || (o->frustumCulled && !camera.frustum.CheckMesh(*o))) continue;

		renderObjects[i].render = true;
		
		// calculate object's apparent Z
		Vector3 v = o->GetWorldPosition();
		camera.matrixView.MultiplyVector3(v);
		renderObjects[i].screenZ = v.z;

		//renderObjects[i].z -= o->zOffset;
	}
}

//////////////////////////////////////////////////////////////////////////
void CScene::CreateSkybox( string folderName )
{
	SAFE_DELETE(pSkybox);	
	
	CArray<CMaterial*> skyboxMat;
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/posx.jpg") );
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/negx.jpg") );
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/posy.jpg") );
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/negy.jpg") );
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/posz.jpg") );
	skyboxMat.AddToTail( new CMaterial( "skybox/"+folderName+"/negz.jpg") );
	
	for (int i=0; i<6; i++) skyboxMat[i]->clamp = true;

	pSkybox = new CMesh(new CCubeGeometry(100,100,100,1,1,1,skyboxMat));
	pSkybox->SetScale(-1,1,1);
	pSkybox->geometry->Initialize();
}

//////////////////////////////////////////////////////////////////////////
void CScene::UpdateObject( CObject3D &object )
{
	RemoveRenderObject(&object);

	if (object.geometry)
	{
		// create geometry groups by material
		object.geometry->Initialize();	

		// add object's geometry groups to the rendering list
		for (int i=0; i<object.geometry->groups.Size(); i++)
		{
			SRenderObject ro = { false, 0.0f,object.geometry->groups[i], (CMesh*)&object };
			renderObjects.AddToTail(ro);			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CScene::Release()
{
	sprites.PurgeAndDeleteElements();
}

//////////////////////////////////////////////////////////////////////////
void CScene::AddSprite( CSprite *spr )
{
	sprites.AddToTail(spr);
}

//////////////////////////////////////////////////////////////////////////
void CScene::RemoveSprite( CSprite *spr )
{
	sprites.Remove(spr);
}

//////////////////////////////////////////////////////////////////////////
void CScene::RemoveSpriteAt( int index )
{
	sprites.RemoveAt(index);
}

//////////////////////////////////////////////////////////////////////////
void CScene::Update( float frametime )
{
	for (int i=0; i<objects.Size(); i++)
	{
		CObject3D *o = objects[i];
		objects[i]->Update(frametime);
	}
}