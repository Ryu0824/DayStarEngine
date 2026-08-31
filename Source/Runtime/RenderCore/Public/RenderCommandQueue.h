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

	void RequestStop();

private:
	FRenderCommandQueue() = default;

	std::queue<FRenderCommand> CommandQueue;
	std::mutex QueueMutex;
	std::condition_variable QueueCondition;
	std::atomic<bool> bIsRunning{ false };
};