#include <gtest/gtest.h>
#include <HAL/FMemory.h>
#include <UObject/Object.h>
#include <UObject/Class.h>
#include <UObject/UObjectGlobals.h>

class CoreUObjectTestFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!GMalloc) FMemory::SetupMemoryPools();
	}
};

class UTestDummyObject :public UObject
{
public:
	int32 Health = 100;

	static UClass* StaticClass()
	{
		static UClass* DummyClass = nullptr;
		if (!DummyClass)
		{
			DummyClass = new UClass();
			DummyClass->Name = TEXT("UTestDummyObject");
			DummyClass->ClassSize = sizeof(UTestDummyObject);
		}

		return DummyClass;
	}
};

TEST_F(CoreUObjectTestFixture, NewObjectAllocationTest)
{
	// New,Delete Func is delete!
	// This Syntax is occured compile error
	// UTestDummyObject* IllegalObj = new UTestDummyObject();

	UTestDummyObject* MyObj = NewObject<UTestDummyObject>();

	EXPECT_NE(MyObj, nullptr);

	EXPECT_EQ(MyObj->Health, 100);

	UClass* ObjClass = MyObj->GetClass();
	EXPECT_NE(ObjClass, nullptr);

	EXPECT_STREQ(*(ObjClass->Name), TEXT("UTestDummyObject"));

	MyObj->~UTestDummyObject();
	FMemory::Free(MyObj);
}