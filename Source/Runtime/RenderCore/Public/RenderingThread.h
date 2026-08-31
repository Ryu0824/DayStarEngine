#pragma once
#include "RenderCommandQueue.h"

extern RENDERCORE_API void StartRenderingThread();
extern RENDERCORE_API void StopRenderingThread();
extern RENDERCORE_API bool IsInRenderingThread();

#define ENQUEUE_RENDER_COMMAND(CommandName) \
	FRenderCommandQueue::Get().EnqueueCommand