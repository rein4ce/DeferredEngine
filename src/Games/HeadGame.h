#pragma once
#include "Game.h"

class CDirectionalLight;

class CHeadGame : public CGame
{
public:
	void Init() override;
	void Update(float frametime, float realtime) override;
	void Render(float frametime, float realtime) override;

private:
	CDirectionalLight *dirLight = nullptr;
};
