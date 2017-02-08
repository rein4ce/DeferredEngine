#pragma once
#include "platform.h"
#include "Resources.h"

enum ECursorTypes
{
	CURSOR_SELECT = 0,
	CURSORS_NUM
};

//////////////////////////////////////////////////////////////////////////
class CCursor
{
public:
	CCursor(void);
	~CCursor(void);

	void		Init(bool v = false);
	void		Release( void );
	void		Render();
	void		Show() { visible = true; };
	void		Hide() { visible = false; };

	CTexture		textures[CURSORS_NUM];
	ECursorTypes	current;
	bool			visible;
};

extern CCursor gCursor;