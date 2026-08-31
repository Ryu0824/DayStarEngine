#include "RenderingThread.h"
#include <thread>

static std::thread* GRenderThread = nullptr;
static std::thread::id GRenderThreadId;

void StartRenderThread()
{
	if (!GRenderThread)
	{
		GRenderThread = new std::thread([]()
			{
				GRenderThreadId = std::this_thread::get_id();

				FRenderCommandQueue::Get().ExcuteCommands();
			}
		)
	}
}

void StopRenderThread()
{
	if (bIsRunning)
	{
		FRenderCommandQueue::Get().RequestStop();

		if (GRenderThread->joinable())
		{
			GRenderThread->join();
		}
		delete GRenderThread;
		GRenderThread = nullptr;
	}
}

bool IsInRenderingThread()
{
	return std::this_thread::get_id() == GRenderThreadId;
}