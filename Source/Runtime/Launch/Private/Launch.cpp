#include "EngineLoop.h"
#include "CoreGlobals.h"

int32 GuardedMain(const TCHAR* CmdLine)
{
	FEngineLoop EngineLoop;

	if (EngineLoop.PreInit(CmdLine) != 0)return -1;
	if (EngineLoop.Init() != 0)return -1;

	while (!GIsRequestingExit)
	{
		EngineLoop.Tick();
	}

	EngineLoop.Exit();
	return 0;
}