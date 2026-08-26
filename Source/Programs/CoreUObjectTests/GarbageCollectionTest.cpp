#include <gtest/gtest.h>
#include "HAL/FMemory.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectArray.h"

class GCTestFixture : public ::testing::Test
{
	virtual void SetUp()override
	{
		if (!GMalloc)
		{
			FMemory::SetupMemoryPools();
		}

		CollectGarbage();
	}
};


class UTestGCObject : public UObject
{
public:
	UTestGCObject* ChildRef = nullptr;

	static UClass* StaticClass()
	{
		static UClass* DummyClass = new UClass();
		DummyClass->Name = TEXT("UTestGCObject");
		DummyClass->ClassSize = sizeof(UTestGCObject);

		return DummyClass;
	}

	virtual void AddReferencedObjects(TArray<UObject*>& OutReachableObjects) override
	{
		if (ChildRef)
		{
			OutReachableObjects.Add(ChildRef);
		}
	}
};

TEST_F(GCTestFixture, MarkAndSweepTest)
{
	UTestGCObject* RootObj = NewObject<UTestGCObject>();
	UTestGCObject* ChildObj = NewObject<UTestGCObject>();
	UTestGCObject* OrphanObj = NewObject<UTestGCObject>();

	RootObj->SetFlags(RF_RootSet);

	RootObj->ChildRef = ChildObj;

	int32 InitialCount = 0;
	for (int32 i = 0;i < GUObjectArray.GetObjectCount();++i)
	{
		if (GUObjectArray.GetItem(i)->Object)InitialCount++;
	}
	EXPECT_EQ(InitialCount, 3);

	CollectGarbage();

	bool bRootAlive = false;
	bool bChildAlive = false;
	bool OrphanAlive = false;

	for (int32 i = 0;i < GUObjectArray.GetObjectCount();++i)
	{
		UObject* Obj = GUObjectArray.GetItem(i)->Object;
		if (Obj == RootObj) bRootAlive = true;
		if (Obj == ChildObj) bChildAlive = true;
		if (Obj == OrphanObj) OrphanAlive = true;
	}

	EXPECT_TRUE(bRootAlive);
	EXPECT_TRUE(bChildAlive);
	EXPECT_FALSE(OrphanAlive);
}