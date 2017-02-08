#pragma once
#include "platform.h"
#include "Utils.h"

struct SFrameGraphItem
{
	string name;
	float start;
	float end;
	float value;
	SRGBA color;
};

class CFrameGraph
{
public:
	CFrameGraph(void);
	~CFrameGraph(void);

	void Reset();
	void Render();

	void Begin(string name, SRGBA color);
	void End(string name);

public:
	int		width;
	std::vector<std::vector<SFrameGraphItem>>	history;
	std::map<string, SFrameGraphItem>			current;
};
