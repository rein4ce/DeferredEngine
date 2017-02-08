#pragma once
#include "platform.h"

using namespace std;

class CResource;

//////////////////////////////////////////////////////////////////////////
class CResourceManager
{
public:
	Singleton(CResourceManager);
	~CResourceManager();

	CResource*	GetResource(string& name);
	CResource*	operator[](string& name);

	void		Map(CResource *res);
	void		Unmap(CResource *res);

	void Release();

public:
	std::map<string, CResource*> pResourceMap;
};

extern CResourceManager gResourceMgr;
