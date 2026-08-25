#include <gtest/gtest.h>
#include "HAL/FMemory.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"

class CoreContainerTestFixture :public ::testing::Test
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

TEST_F(CoreContainerTestFixture, MemoryUtilitiesTest)
{
	int32 Buffer[5];

	FMemory::Memzero(Buffer, sizeof(Buffer));
	EXPECT_EQ(Buffer[0], 0);
	EXPECT_EQ(Buffer[4], 0);

	int32 Src[5] = { 1,2,3,4,5 };
	FMemory::Memcpy(Buffer, Src, sizeof(Src));
	EXPECT_EQ(Buffer[0], 1);
	EXPECT_EQ(Buffer[4], 5);
}

TEST_F(CoreContainerTestFixture, ArrayTrivialTypeTest)
{
	TArray<int32> Arr;
	Arr.Add(10);
	Arr.Add(20);
	Arr.Add(30);

	EXPECT_EQ(Arr.Num(), 3);
	EXPECT_EQ(Arr.GetData()[1], 20);

	Arr.Empty();
	EXPECT_EQ(Arr.Num(), 0);
}

static int32 GDestructorCallCount = 0;
struct FNonTrivialStruct
{
	FNonTrivialStruct() {}
	~FNonTrivialStruct() { GDestructorCallCount++; }
};

TEST_F(CoreContainerTestFixture, ArrayNonTrivialTypeTest)
{
	GDestructorCallCount = 0;

	{
		TArray<FNonTrivialStruct> Arr;
		Arr.Add(FNonTrivialStruct());
		Arr.Add(FNonTrivialStruct());

		GDestructorCallCount = 0;
		Arr.Empty();

		EXPECT_EQ(GDestructorCallCount, 2);
	}
}

TEST_F(CoreContainerTestFixture, UnrealStringTEST)
{
	FString Str(TEXT("DayStar"));

	EXPECT_EQ(Str.Len(), 7);

	const TCHAR* CStr = *Str;
	EXPECT_EQ(CStr[0], TEXT("D"));
	EXPECT_EQ(CStr[6], TEXT("r"));
	EXPECT_EQ(CStr[7], 0);
}