#include <gtest/gtest.h>
#include "GenericPlatform/GenericApplication.h"
#include "GenericPlatform/GenericWindow.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

class ApplicationCoreTestFixture :public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!GMalloc)
		{
			FMemory::SetupMemoryPools();
		}
	}
};

class FMockMessageHandler : public FGenericApplicationMessageHandler
{
public:
	bool bReceivedKeyDown = false;
	int32 LastKeyCode = -1;
	bool bReceivedWindowClose = false;

	virtual bool OnKeyDown(int32 KeyCode, uint32 CharacterCode, bool bIsRpeat) override
	{
		bReceivedKeyDown = true;
		LastKeyCode = KeyCode;
		return true;
	}

	virtual void OnWindowClose() override
	{
		bReceivedWindowClose = true;
	}
};

class FMockWindow : public FGenericWindow
{
public:
	virtual void Show() override {}
	virtual void Hide() override {}
	virtual void* GetOSWindowHandle() const override { return (void*)0xDEADBEEF; }
};

class FMockApplication : public FGenericApplication
{
public:
	virtual void PumpMessages(const float TimeDelta) override
	{
		if (MessageHandler)
		{
			MessageHandler->OnKeyDown(42, 0, false);
			MessageHandler->OnWindowClose();
		}
	}

	virtual TSharedPtr<FGenericWindow> MakeWindow(const FGenericWindowDefinition& Definition) override
	{
		return MakeShared<FMockWindow>();
	}
};

TEST_F(ApplicationCoreTestFixture, WindowCreationAndHandle)
{
	TSharedPtr<FMockApplication> App = MakeShared<FMockApplication>();
	FGenericWindowDefinition Def;
	TSharedPtr<FGenericWindow> Window = App->MakeWindow(Def);

	EXPECT_NE(Window, nullptr);
	EXPECT_EQ(Window->GetOSWindowHandle(), (void*)0xDEADBEEF);

	FGenericApplicationMessageHandler* a = new FMockMessageHandler();
}

TEST_F(ApplicationCoreTestFixture, EventRouting)
{
	TSharedPtr<FMockApplication> App = MakeShared<FMockApplication>();
	TSharedPtr<FMockMessageHandler> Handler = MakeShared<FMockMessageHandler>();

	App->SetMessageHandler(Handler);

	EXPECT_FALSE(Handler->bReceivedKeyDown);
	EXPECT_FALSE(Handler->bReceivedWindowClose);

	App->PumpMessages(0.016f);

	EXPECT_TRUE(Handler->bReceivedKeyDown);
	EXPECT_EQ(Handler->LastKeyCode, 42);
	EXPECT_TRUE(Handler->bReceivedWindowClose);
}