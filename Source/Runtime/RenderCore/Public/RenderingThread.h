#pragma once
#include "RenderCommandQueue.h"
#include "RHICommandList.h"
#include <utility>

class FRenderCommand
{
public:
	virtual ~FRenderCommand() = default;

	virtual void DoTask(FRHICommandListImmediate& RHICmdList) = 0;
	virtual const TCHAR* GetDebugName() const = 0;
};

template<typename TStatId, typename TLambda>
class TLambdaRenderCommand : public FRenderCommand
{
	TLambda Lambda;
public:
	TLambdaRenderCommand(TLambda&& InLambda) : Lambda(std::move(InLambda)) {}

	virtual void DoTask(FRHICommandListImmediate& RHICmdList) override
	{
		Lambda(RHICmdList);
	}
	virtual const TCHAR* GetDebugName() const override { return TStatId::Name(); }
};

template<typename TStatId>
struct FEnqueueCommandHelper
{
	template<typename TLambda>
	void operator()(TLambda&& Lambda)
	{
		FRenderCommandQueue::Get().EnqueueCommand(new TLambdaRenderCommand<TStatId, TLambda>(std::forward<TLambda>(Lambda)));
	}
};

extern RENDERCORE_API void StartRenderingThread();
extern RENDERCORE_API void StopRenderingThread();
extern RENDERCORE_API bool IsInRenderingThread();

#define ENQUEUE_RENDER_COMMAND(CommandName) \
	struct CommandName##_StatId \
	{\
		static const TCHAR* Name() { return TEXT(#CommandName);}\
	};\
	FEnqueueCommandHelper<CommandName##_StatId>()