#pragma once

#include "platform.h"

class CView
{
public:
	CView();
	~CView();

	void Set( float newx, float newy, float newzoom=1.0f );
	void UpdateViewport( int w, int h );

	void Screen2Map( int sx, int sy, float &mapx, float &mapy );
	void Map2Screen( float mapx, float mapy, int &sx, int &sy );

	CView operator = (CView &v);
	
public:
	float				x;							// Parametry aktualnego widoku
	float				y;
	int					width;
	int					height;
	float				aspect;
	float				zoom;
};

inline CView::CView()
{
	x = 0.0f;
	y = 0.0f;
	zoom = 0.0f;
	width = height = 0;
	aspect = 0.0f;
}

inline CView::~CView()
{
}

inline void CView::UpdateViewport(int w, int h)
{
	if (h==0)	h=1;
	
	width = w;
	height = h;
	aspect = (float)w/(float)h;
}

inline void CView::Set(float newx, float newy, float newzoom )
{
	x = newx;
	y = newy;
	zoom = newzoom;
}

inline CView CView::operator = ( CView &v)
{
	x = v.x;
	y = v.y;
	zoom = v.zoom;

	return *this;
}

inline void CView::Screen2Map( int sx, int sy, float &mapx, float &mapy )
{
	mapx = sx-(width/2)+x;
	mapy = sy-(height/2)+y;
}

inline void CView::Map2Screen( float mapx, float mapy, int &sx, int &sy )
{
	sx = -mapx+(width/2)-x;
	sy = -mapy+(height/2)-y;
}