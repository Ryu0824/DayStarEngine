#include <gtest/gtest.h>
#include "EngineLoop.h"
#include "CoreGlobals.h"
#include "RenderingThread.h"

TEST(LaunchTest, EngineLoopLifeCycle)
{
	FEngineLoop EngineLoop;

	EXPECT_EQ(EngineLoop.PreInit(TEXT("-game -log")), 0) << "PreInit failed!";

	EXPECT_EQ(EngineLoop.Init(), 0) << "Init Failed!";
	EXPECT_TRUE(IsInRenderingThread() == false) << "Main thread confused with Render Thread!";

	int32 TickCount = 0;
	while (!GIsRequestingExit)
	{
		EngineLoop.Tick();
		TickCount++;

		if (TickCount >= 3)
		{
			GIsRequestingExit = true;
		}
	}

	EXPECT_EQ(TickCount, 3) << "Engine did not tick exactly 3 times before exit request!";

	EngineLoop.Exit();

	GIsRequestingExit = false;
}