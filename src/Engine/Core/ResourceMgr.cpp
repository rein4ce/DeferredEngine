#include "platform.h"
#include "ResourceMgr.h"
#include <shlwapi.h>
#include "Utils.h"
#include "Resources.h"

CResourceManager gResourceMgr = *CResourceManager::Instance();

CResourceManager::CResourceManager(void)
{
}

CResourceManager::~CResourceManager(void)
{
	trace("Deleting ResourceManager");	
}

CResource* CResourceManager::GetResource(string& name)
{
	std::map<string, CResource*>::iterator it = pResourceMap.find(name);
	if (it == pResourceMap.end()) return NULL;
	return it->second;
}

CResource* CResourceManager::operator[](string& name)
{
	return GetResource(name);
}

void CResourceManager::Release()
{
	trace("CResourceManager Release()");
	for (std::map<string, CResource*>::iterator it = pResourceMap.begin(); it != pResourceMap.end(); it++)
	{
		CResource *res = it->second;
		if (res)
		{
			res->Release();			
		}
	}
	pResourceMap.clear();		// destructors will be called
}

void CResourceManager::Map( CResource *res )
{
	pResourceMap[res->name] = res;
}

void CResourceManager::Unmap( CResource *res )
{
	std::map<string, CResource*>::iterator it = pResourceMap.find(res->name);
	if (it != pResourceMap.end())
		pResourceMap.erase(it);
}
