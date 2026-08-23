#include <gtest/gtest.h>
#include "HAL/FMemory.h"
#include "Misc/AssertionMacros.h"

class CoreTestFixture : public ::testing::Test
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

TEST_F(CoreTestFixture, MemoryAllocationTest)
{
	const int32 BufferSize = 256;
	void* Ptr = FMemory::Malloc(BufferSize);

	EXPECT_NE(Ptr, nullptr);

	uint8* ByteBuffer = static_cast<uint8*>(Ptr);
	for (int32 i = 0;i < BufferSize;++i)
	{
		ByteBuffer[i] = static_cast<uint8>(i % 256);
	}

	EXPECT_EQ(ByteBuffer[10], 10);

	FMemory::Free(Ptr);
}

TEST_F(CoreTestFixture, AssertionDeathTest)
{
	EXPECT_DEATH({
		int32 InvalidIndex = -1;
	check(InvalidIndex >= 0);
		}, "");
}