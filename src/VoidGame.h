#pragma once

class ID3D10Device;
class CVar;

extern ID3D10Device*				g_pDevice;
extern CVar cv_server;

#define PRINTF						gVGUI.AddTextMessage
