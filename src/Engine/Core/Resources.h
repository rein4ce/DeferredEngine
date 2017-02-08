#pragma once
#include "platform.h"

using namespace std;

#pragma comment(lib, "3rdparty/devil/DevIL.lib")
#pragma comment(lib, "3rdparty/devil/ILU.lib")
#pragma comment(lib, "3rdparty/devil/ILUT.lib")

class CImage;
class CTexture;
class CCubeTexture;
struct SRGBA;

class CResource 
{
public:
public:
	CResource();
	virtual ~CResource();

	virtual void			Release();
	virtual bool			Load(string filename) = 0;		

protected:
	void			Map(string name);
	void			Unmap();

public:
	bool			loaded;
	string			name;	
};



//////////////////////////////////////////////////////////////////////////
class CTexture : public CResource
{
public:
	CTexture();
	CTexture(string filename);
	CTexture(string name, string filename);
	CTexture(CImage *image);

	virtual void Release();
	virtual bool Load(string filename);	

	void FromMemory( string name, void *pData, uint32 size );
	bool Create(CImage *image);

	static CTexture*	Get(string filename);

	ID3D10ShaderResourceView*	GetTexture() { return pTextureView; }

public:
	ID3D10Texture2D						*pTexture;
	ID3D10ShaderResourceView			*pTextureView;	

	int			width, height;
};


//////////////////////////////////////////////////////////////////////////
class CCubeTexture : public CResource
{
public:
	CCubeTexture(string filename);

	virtual void Release();
	virtual bool Load(string filename);	

public:
	ID3D10ShaderResourceView			*pTexture;
};


//////////////////////////////////////////////////////////////////////////
// Image, can be used for textures
//////////////////////////////////////////////////////////////////////////
class CImage : public CResource
{
public:
	CImage();
	CImage(string filename);

	virtual void Release();
	virtual bool Load(string filename);

	void	Create(int width, int height, SRGBA *data = NULL);
	SRGBA	GetPixel(int x, int y);

public:
	int		width, height;
	byte	*pData;
	int		idImage;
};


