#include "EngineLoop.h"
#include "CoreGlobals.h"
#include "RenderingThread.h"
#include "DynamicRHI.h"
#include "Misc/AssertionMacros.h"

int32 FEngineLoop::PreInit(const TCHAR* CmdLine)
{
	return 0;
}

int32 FEngineLoop::Init()
{
	FGenericWindowDefinition WindowDef;
	WindowDef.Title = TEXT("DayStar Engine");

	StartRenderingThread();

	return 0;
}

void FEngineLoop::Tick()
{
	if (Application.IsValid())
	{
		Application->PumpMessages(0.016f);
	}

	// Game Logic Update

	ENQUEUE_RENDER_COMMAND(EngineLoopRenderTick)(
		[](FRHICommandListImmediate& RHICmdList)
		{
			// Backend Draw Call
		}
		);
}

void FEngineLoop::Exit()
{
	StopRenderingThread();

	MainWindow.Reset();
	Application.Reset();
}