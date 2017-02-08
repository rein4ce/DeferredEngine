#include "platform.h"
#include "FrameGraph.h"
#include "CVar.h"
#include "Renderer.h"
#include "Engine.h"
#include "GeometryRenderer.h"
#include "VGUI.h"

CVar cv_graph("graph", "1", FCVAR_ARCHIVE);
CVar cv_graphScale("graphscale", "100.0", FCVAR_ARCHIVE);

CFrameGraph::CFrameGraph(void)
{
	width = 200;	
}

CFrameGraph::~CFrameGraph(void)
{
}

void CFrameGraph::Reset()
{
	history.clear();
	current.clear();
}

void CFrameGraph::Render()
{
	if (!cv_graph.GetBool()) return;

	int marginX = 10;
	int marginY = 40;
	int x = gEngine.width - marginX - width;
	int y = gEngine.height - marginY;
	x += width - history.size();

	gRenderer.pGeometryRenderer->ortho = true;

	// Add current measurements to the history
	std::vector<SFrameGraphItem> item;
	std::map<string, SFrameGraphItem>::iterator iter;
	int i = 1;
	for (iter = current.begin(); iter != current.end(); iter++)	
	{
		item.push_back(iter->second);
		PrintText(gEngine.width - marginX - width - 60, y - i*12, iter->second.color, iter->second.name.c_str());
		i++;
	}
	history.push_back(item);
	if (history.size() > width) history.erase(history.begin());
	current.clear();

	// Draw the graph
	for (int i=0; i<history.size(); i++)
	{
		int cy = y;
		for (int j=0; j<history[i].size(); j++)
		{
			int ey = cy - history[i][j].value * cv_graphScale.GetFloat(); 			
			gRenderer.pGeometryRenderer->DrawLine(
				Vector3(x+i, cy, 0), 
				Vector3(x+i, ey, 0),
				history[i][j].color);
			cy = ey;
		}
	}	

	// Baseline
	x = gEngine.width - marginX - width;
	gRenderer.pGeometryRenderer->DrawLine(
		Vector3(x,y,0),
		Vector3(x+width,y,0),
		SRGBA(255,255,255,100));
		
	gRenderer.pGeometryRenderer->Render();
}

void CFrameGraph::Begin( string name, SRGBA color )
{
	SFrameGraphItem item;
	item.name = name;
	item.start = GetFloatTime();
	item.color = color;
	current[name] = item;
}

void CFrameGraph::End( string name )
{
	std::map<string, SFrameGraphItem>::iterator iter = current.find(name);
	Assert(iter != current.end());
	iter->second.end = GetFloatTime();
	iter->second.value = iter->second.end - iter->second.start;
}