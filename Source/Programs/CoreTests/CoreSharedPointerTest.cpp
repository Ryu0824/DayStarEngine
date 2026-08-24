#include <gtest/gtest.h>
#include "HAL/FMemory.h"
#include "Templates/SharedPointer.h"

class CoreSharedPointerTestFixture : public ::testing::Test
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

static int32 GDestructorCount = 0;

struct FTestObject
{
	int32 Value;
	FTestObject(int32 InValue) :Value(InValue) {}
	~FTestObject() { GDestructorCount++; }
};

TEST_F(CoreSharedPointerTestFixture, SharedPointerLifetimeTest)
{
	GDestructorCount = 0;
	{
		TSharedRef<FTestObject> SharedRef(new FTestObject(42));
		EXPECT_EQ(SharedRef->Value, 42);

		{
			TSharedPtr<FTestObject> SharedPtr1 = SharedRef;
			TSharedPtr<FTestObject> SharedPtr2 = SharedPtr1;
			
			EXPECT_TRUE(SharedPtr2.IsValid());
			EXPECT_EQ(GDestructorCount, 0);
		}
	}
	EXPECT_EQ(GDestructorCount, 1);
}

TEST_F(CoreSharedPointerTestFixture, WeakPointerPinTest)
{
	TWeakPtr<FTestObject> WeakPtr;
	{
		TSharedPtr<FTestObject> SharedPtr(TSharedRef<FTestObject>(new FTestObject(100)));
		WeakPtr = SharedPtr;

		EXPECT_TRUE(WeakPtr.IsValid());
		TSharedPtr<FTestObject> PinnedPtr = WeakPtr.Pin();
		EXPECT_TRUE(PinnedPtr.IsValid());
		EXPECT_EQ(PinnedPtr->Value, 100);
	}

	EXPECT_FALSE(WeakPtr.IsValid());
	TSharedPtr<FTestObject> FailedPin = WeakPtr.Pin();
	EXPECT_FALSE(FailedPin.IsValid());
}