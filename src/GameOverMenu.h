#pragma once
#include "platform.h"

class CSprite2D;

class CGameOverMenu
{
public:
	CGameOverMenu(void);
	~CGameOverMenu(void);

	void Show();
	void Hide();

	void Update(float frametime);

public:
	CSprite2D		*background;
	float			opacity;
	bool			showing;
	float			showTime;
};
