#include <gtest/gtest.h>
#include "Templates/RefCountPtr.h"
#include "RenderingThread.h"
#include "RHIResources.h"

class FTestRHIResrouce : public FRHIResource
{
public:
	bool* bDestroyed;
	FTestRHIResrouce(bool* InDestroyed) : bDestroyed(InDestroyed) { *bDestroyed = false; }
	virtual ~FTestRHIResrouce() override { *bDestroyed = true; }
};

TEST(RenderCoreTest, RefCountPtrLifecycle)
{
	bool bIsDestroyed = false;
	{
		TRefCountPtr<FTestRHIResrouce> Ptr(new FTestRHIResrouce(&bIsDestroyed));
		EXPECT_EQ(Ptr->GetRefCount(), 1);

		{
			TRefCountPtr<FTestRHIResrouce> Copy = Ptr;
			EXPECT_EQ(Ptr->GetRefCount(), 2);
		}
		EXPECT_EQ(Ptr->GetRefCount(), 1);

		TRefCountPtr<FTestRHIResrouce> Moved = std::move(Ptr);
		EXPECT_FALSE(Ptr.IsValid());
		EXPECT_EQ(Moved->GetRefCount(), 1);
	}

	EXPECT_TRUE(bIsDestroyed);
}

TEST(RenderCoreTest, EnqueueRenderCommandExecution)
{
	StartRenderingThread();

	std::atomic<bool> bTaskExecuted{ false };
	std::atomic<bool> bWasInRenderThread{ false };
	std::thread::id MainThreadId = std::this_thread::get_id();
	std::thread::id TaskThreadId;

	ENQUEUE_RENDER_COMMAND(FTestTaskCommand)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			TaskThreadId = std::this_thread::get_id();
			bWasInRenderThread = IsInRenderingThread();
			bTaskExecuted = true;
		});

	StopRenderingThread();

	EXPECT_TRUE(bTaskExecuted) << "Render command was never executed!";
	EXPECT_TRUE(bWasInRenderThread) << "Task did not recognize it was in the Render Thread!";
	EXPECT_NE(MainThreadId, TaskThreadId) << " Task was executed synchronously on the Main Thread!";
}