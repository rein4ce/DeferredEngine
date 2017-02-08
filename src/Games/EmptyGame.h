#pragma once
#include "Game.h"
#include <vector>

class CLight;

class CEmptyGame : public CGame
{
public:
	CEmptyGame();
	virtual ~CEmptyGame();

	virtual	void		Init();
	virtual void		Update(float frametime, float realtime);
	virtual void		Reset();

private:
	std::vector<CLight*>	lights;
};