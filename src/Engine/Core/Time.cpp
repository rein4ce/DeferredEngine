#include "platform.h"
#include "platform.h"

LARGE_INTEGER g_PerformanceFrequency;
LARGE_INTEGER g_MSPerformanceFrequency;
LARGE_INTEGER g_ClockStart;

void InitTime()
{
	if( !g_PerformanceFrequency.QuadPart )
	{
		QueryPerformanceFrequency(&g_PerformanceFrequency);
		g_MSPerformanceFrequency.QuadPart = g_PerformanceFrequency.QuadPart / 1000;
		QueryPerformanceCounter(&g_ClockStart);
	}
}

double GetFloatTime()
{
	InitTime();
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter( &CurrentTime );
	double fRawSeconds = (double)( CurrentTime.QuadPart - g_ClockStart.QuadPart ) / (double)(g_PerformanceFrequency.QuadPart);
	return fRawSeconds;
}