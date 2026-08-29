#include "RenderCommandQueue.h"

FRenderCommandQueue& FRenderCommandQueue::Get()
{
	static FRenderCommandQueue Instance;
	return Instance;
}

void FRenderCommandQueue::EnqueueCommand(FRenderCommand&& Command)
{
	{
		std::lock_guard<std::mutex> Lock(QueueMutex);
		CommandQueue.push(std::move(Command));
	}

	QueueCondition.notify_one();
}

void FRenderCommandQueue::ExcuteCommands()
{
	while (bIsRunning)
	{
		FRenderCommand Task;
		{
			std::unique_lock<std::mutex> Lock(QueueMutex);

			QueueCondition.wait(Lock, [this]() {return !CommandQueue.empty() || !bIsRunning;});

			if (!bIsRunning && CommandQueue.empty())
				break;

			Task = std::move(CommandQueue.front());
			CommandQueue.pop();
		}

		if (Task)
		{
			Task();
		}
	}
}

void FRenderCommandQueue::StartRenderThread()
{
	if (!bIsRunning)
	{
		bIsRunning = true;
		RenderThreadHandle = new std::thread(&FRenderCommandQueue::ExcuteCommands, this);
	}
}

void FRenderCommandQueue::StopRenderThread()
{
	if (bIsRunning)
	{
		bIsRunning = false;
		QueueCondition.notify_all();

		if (RenderThreadHandle && RenderThreadHandle->joinable())
		{
			RenderThreadHandle->join();
		}
		delete RenderThreadHandle;
		RenderThreadHandle = nullptr;
	}
}