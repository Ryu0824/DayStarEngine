#include "RenderCommandQueue.h"
#include "RenderingThread.h"

FRenderCommandQueue& FRenderCommandQueue::Get()
{
	static FRenderCommandQueue Instance;
	return Instance;
}

void FRenderCommandQueue::EnqueueCommand(FRenderCommand* Command)
{
	{
		std::lock_guard<std::mutex> Lock(QueueMutex);
		CommandQueue.push(Command);
	}
	QueueCondition.notify_one();
}

void FRenderCommandQueue::ExcuteCommands()
{
	FRHICommandListImmediate RHICmdList;

	while (true)
	{
		FRenderCommand* Task = nullptr;
		{
			std::unique_lock<std::mutex> Lock(QueueMutex);
			QueueCondition.wait(Lock, [this]() {return !CommandQueue.empty() || !bIsRunning;});

			if (!bIsRunning && CommandQueue.empty()) break;

			Task = CommandQueue.front();
			CommandQueue.pop();
		}

		if (Task)
		{
			Task->DoTask(RHICmdList);

			delete Task;
		}
	}
}

void FRenderCommandQueue::RequestStop()
{
	bIsRunning = false;
	QueueCondition.notify_all();
}