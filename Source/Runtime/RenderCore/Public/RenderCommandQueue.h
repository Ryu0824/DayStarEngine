#pragma once
#include "RenderCoreAPI.h"
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <atomic>

using FRenderCommand = std::function<void()>;

class RENDERCORE_API FRenderCommandQueue
{
public:
	static FRenderCommandQueue& Get();

	void EnqueueCommand(FRenderCommand&& Command);

	void ExcuteCommands();

	void StartRenderThread();
	void StopRenderThread();

private:
	FRenderCommandQueue() = default;

	std::queue<FRenderCommand> CommandQueue;
	std::mutex QueueMutex;
	std::condition_variable QueueCondition;

	std::atomic<bool> bIsRunning{ false };
	class std::thread* RenderThreadHandle = nullptr;

};
	
#define ENQUEUE_RENDER_COMMAND(CommandName) \ 
	FRenderCommandQueue::Get().EnqueueCommand