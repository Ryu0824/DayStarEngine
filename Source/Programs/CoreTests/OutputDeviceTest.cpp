#include <gtest/gtest.h>
#include "MISC/OutputDeviceRedirector.h"
#include "MISC/OutputDeviceConsole.h"
#include "Logging/LogMacros.h"
#include "Containers/Array.h"
#include "Containers/Unrealstring.h"

class FMockOutputDevice :public FOutputDevice
{
public:
	FString LastMessage;
	ELogVerbosity::Type LastVerbosity;
	FString LastCategory;

	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category)
	{
		LastMessage = Message;
		LastVerbosity = Verbosity;
		LastCategory = Category.CategoryName;
	}
};

class OutputDeviceTestFixture :public ::testing::Test
{
protected:
	FMockOutputDevice* MockDevice;

	void SetUp() override
	{
		if (!GMalloc)
		{
			FMemory::SetupMemoryPools();
		}

		GLog = new FOutputDeviceRedirector();

		MockDevice = new FMockOutputDevice();

		GLog->AddOutputDevice(MockDevice);
	}

	void TearDown()override
	{
		delete MockDevice;
		delete GLog;
		GLog = nullptr;
	}
};

DECLARE_LOG_CATEGORY_EXTERN(OutputLog, Log);
DEFINE_LOG_CATEGORY(OutputLog);

TEST_F(OutputDeviceTestFixture, GLogBroadcastTest)
{
	DS_LOG(OutputLog, ELogVerbosity::Display, TEXT("Output Device Test %d"), 100);

	EXPECT_STREQ(*MockDevice->LastMessage, TEXT("Output Device Test 100"));
	EXPECT_EQ(MockDevice->LastVerbosity, ELogVerbosity::Display);
	EXPECT_STREQ(*MockDevice->LastCategory, TEXT("OutputLog"));
}