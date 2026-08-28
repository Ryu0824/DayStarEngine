#include <gtest/gtest.h>
#include "MISC/OutputDeviceRedirector.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

class ModuleTestFixture :public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!GMalloc)
		{
			FMemory::SetupMemoryPools();
		}

		GLog = new FOutputDeviceRedirector();
	}

	void TearDown()override
	{
		delete GLog;
		GLog = nullptr;
	}
};

class FMockTestModule : public IModuleInterface
{
public:
	bool bStartUpCalled = false;
	bool bShutdownCalled = false;

	virtual void StartupModule() override
	{
		bStartUpCalled = true;
	}

	virtual void ShutdownModule() override
	{
		bShutdownCalled = true;
	}
};


TEST_F(ModuleTestFixture, InvalidModuleLoadFailsGracefully)
{
	IModuleInterface* FailedModule = FModuleManager::Get().LoadModule(TEXT("NoExistent_Core.dll"));

	EXPECT_EQ(FailedModule, nullptr);
	EXPECT_EQ(FModuleManager::Get().GetModule(TEXT("NoExistent_Core.dll")), nullptr);
}

TEST_F(ModuleTestFixture, ModuleLifecycleExecution)
{
	FMockTestModule MockModule;

	EXPECT_FALSE(MockModule.bStartUpCalled);
	EXPECT_FALSE(MockModule.bShutdownCalled);

	MockModule.StartupModule();
	EXPECT_TRUE(MockModule.bStartUpCalled);
	EXPECT_FALSE(MockModule.bShutdownCalled);

	MockModule.ShutdownModule();
	EXPECT_TRUE(MockModule.bShutdownCalled);
}