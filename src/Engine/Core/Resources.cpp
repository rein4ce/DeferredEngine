#include "platform.h"
#include "Resources.h"
#include "Direct3D.h"
#include "3rdparty/devil/IL/il.h"
#include "ResourceMgr.h"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////////
CResource::CResource()
{
	loaded = false;
}

void CResource::Map(string name)
{
	loaded = true;
	this->name = name;
	gResourceMgr.Map(this);
}

void CResource::Unmap()
{
	if (loaded)
	{
		gResourceMgr.Unmap(this);
		loaded = false;
	}	
}

void CResource::Release()
{
	Unmap();
}

CResource::~CResource()
{
	Release();
}


//////////////////////////////////////////////////////////////////////////
CTexture::CTexture() 
{
	pTextureView = NULL;
	pTexture = NULL;
}

CTexture::CTexture(string filename)
{
	pTextureView = NULL;
	pTexture = NULL;
	Load(filename);
}

void CTexture::Release()
{
	CResource::Release();
	SAFE_RELEASE_DELETE(pTextureView);
	SAFE_RELEASE(pTexture);
}	

bool CTexture::Load(string filename)
{
	Release();

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	loadInfo.MipLevels = 1;
	loadInfo.MipFilter = D3DX10_FILTER_NONE;	// TODO: temp
	
	HRESULT result;	
	if (FAILED(result = D3DX10CreateShaderResourceViewFromFile(Direct3D::Instance()->GetDevice(), filename.c_str(), &loadInfo, NULL, &pTextureView, NULL)))
	{
		// load null texture
		D3DX10CreateShaderResourceViewFromFile(Direct3D::Instance()->GetDevice(), "textures/null.png", NULL, NULL, &pTextureView, NULL);
		trace("Error while loading texture from file %s: %d", filename, result);
		return false;
	}

	ID3D10Resource* resource;
	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	pTextureView->GetResource(&resource);
	pTextureView->GetDesc(&viewDesc);
	D3D10_TEXTURE2D_DESC desc2D;
	((ID3D10Texture2D*)resource)->GetDesc(&desc2D);

	this->width = desc2D.Width;
	this->height = desc2D.Height;

	Map(filename);
	return true;
}

void CTexture::FromMemory( string name, void *pData, uint32 size )
{
	HRESULT result;
	if (FAILED(result = D3DX10CreateShaderResourceViewFromMemory(Direct3D::Instance()->GetDevice(), pData, size, NULL, NULL, &pTextureView, NULL)))
	{
		// load null texture
		D3DX10CreateShaderResourceViewFromFile(Direct3D::Instance()->GetDevice(), "textures/null.png", NULL, NULL, &pTextureView, NULL);
		trace("Error while loading texture from memory: %d", result);			
		return;
	}

	Map(name);
}

CTexture::CTexture( CImage *image )
{
	Create(image);
}

bool CTexture::Create(CImage *image)
{
	// texture description
	D3D10_TEXTURE2D_DESC descTex;
	descTex.Width = image->width;
	descTex.Height = image->height;
	descTex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descTex.Usage = D3D10_USAGE_DEFAULT;
	descTex.BindFlags = D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
	descTex.CPUAccessFlags = 0;
	descTex.MipLevels = 1;
	descTex.ArraySize = 1;
	descTex.SampleDesc.Count = 1;
	descTex.SampleDesc.Quality = 0;
	descTex.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;

	// resource data descriptor
	D3D10_SUBRESOURCE_DATA data ;
	memset( &data, 0, sizeof(D3D10_SUBRESOURCE_DATA));
	data.pSysMem = image->pData;
	data.SysMemPitch = 4 * image->width;

	// Create the 2d texture from data	
	HRESULT hr;
	if (FAILED( hr = gD3D->GetDevice()->CreateTexture2D( &descTex, &data, &pTexture )))
	{
		Error("Error while creating texture from image: %d", hr);
		return false;
	}

	// Create resource view descriptor
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = descTex.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = D3D10_RESOURCE_MISC_GENERATE_MIPS ;

	// Create the shader resource view
	if (FAILED( hr = gD3D->GetDevice()->CreateShaderResourceView( pTexture, &srvDesc, &pTextureView )))
	{
		Error("Error while creating texture shader resource view from image: %d", hr);
		return false;
	}
	
	Map(image->name);
	return true;
}

CTexture* CTexture::Get(string filename)
{
	CResource *res = gResourceMgr.GetResource(filename);
	if (!res)
	{
		CTexture *tex = new CTexture(filename);
		return tex;
	}
	return (CTexture*)res;
}

//////////////////////////////////////////////////////////////////////////
CCubeTexture::CCubeTexture(string filename)
{
	pTexture = NULL;
	Load(filename);
}

void CCubeTexture::Release()
{
	CResource::Release();
	SAFE_RELEASE_DELETE(pTexture);
}

bool CCubeTexture::Load(string filename)
{
	Release();
	
	D3DX10_IMAGE_LOAD_INFO loadInfo;
	loadInfo.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

	// load texture file
	ID3D10Texture2D* tex = NULL;
	HRESULT result;
	if (FAILED(result = D3DX10CreateTextureFromFile(Direct3D::Instance()->GetDevice(), filename.c_str(), &loadInfo, NULL, (ID3D10Resource**)&tex, NULL)))
	{
		Error("Error while loading cube texture %s:\n%d", filename.c_str(), result);
		return false;
	}

	// create cube texture resource view
	D3D10_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	viewDesc.TextureCube.MostDetailedMip = 0;

	if (FAILED(result = Direct3D::Instance()->GetDevice()->CreateShaderResourceView(tex, &viewDesc, &pTexture)))
	{
		Error("Error while creating cube texture %s shader resource view:\n%d", filename.c_str(), result);
		return false;
	}

	tex->Release();
	Map(filename);
	return true;
}


//////////////////////////////////////////////////////////////////////////
CImage::CImage() 
{
	idImage = -1;
	pData = NULL;
}

CImage::CImage(string filename) 
{
	idImage = -1;
	pData = NULL;
	Load(filename);
}

bool CImage::Load(string filename)
{	
	Release();

	idImage = ilGenImage();
	ilBindImage(idImage);
	ilLoadImage(filename.c_str());

	if (ilGetError() != IL_NO_ERROR)
	{
		Error("Error while loading image '%s': %d", filename.c_str(), ilGetError());
		return false;
	}

	// get dimensions
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);

	// load the data
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	pData = ilGetData();

	Map(filename);
}

void CImage::Release()
{
	CResource::Release();
	width = height = 0;
	SAFE_DELETE(pData);
	if (idImage != -1)
	{
		ILuint id = idImage;
		ilDeleteImages( 1, &id );
		idImage = -1;
	}
}

SRGBA CImage::GetPixel( int x, int y )
{
	if (x < 0 || y < 0 || x >= width || y >= height || !loaded) return BLACK;
	SRGBA c = SRGBA();
	c.r = pData[(y * width + x)*4 + 0];
	c.g = pData[(y * width + x)*4 + 1];
	c.b = pData[(y * width + x)*4 + 2];
	c.a = pData[(y * width + x)*4 + 3];
	return c;
}

void CImage::Create( int width, int height, SRGBA *data /*= NULL*/ )
{	
	Release();

	this->width = width;
	this->height = height;

	pData = new byte[width * height * 4];
	for (int i=0; i<width*height; i++)
	{
		if (data != NULL)
		{
			pData[i*4+0] = data[i].r;
			pData[i*4+1] = data[i].g;
			pData[i*4+2] = data[i].b;
			pData[i*4+3] = data[i].a;
		}
		else
		{
			pData[i*4+0] = 0;
			pData[i*4+1] = 0;
			pData[i*4+2] = 0;
			pData[i*4+3] = 0;
		}
	}

	loaded = true;
}

