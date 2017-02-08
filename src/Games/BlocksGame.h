#pragma once
#include "platform.h"
#include "Game.h"

class CBlocksGame : public CGame
{
public:
	CBlocksGame();
	virtual ~CBlocksGame();

	virtual void			Init();
	virtual void			Reset();

	void					BlockWorldUpdate(float frametime, float realtime);
	virtual void			Update(float frametime, float realtime);
};