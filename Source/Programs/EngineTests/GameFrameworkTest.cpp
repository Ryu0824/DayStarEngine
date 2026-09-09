#include <gtest/gtest.h>
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

class GameFrameWorkTestFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!GMalloc) FMemory::SetupMemoryPools();
	}
};

class UMockComponent :public UActorComponent
{
public:
	static UClass* StaticClass()
	{
		static UClass* DummyClass;
		if (!DummyClass)
		{
			DummyClass = new UClass;
			DummyClass->Name = TEXT("UMockComponentClass");
			DummyClass->ClassSize = sizeof(UMockComponent);
		}

		return DummyClass;
	}

	bool bBeginPlayCalled = false;
	bool bEndPlayCalled = false;
	int32 TickCount = 0;

	virtual void BeginPlay() override
	{
		UActorComponent::BeginPlay();
		bBeginPlayCalled = true;
	}

	virtual void TickComponent(float DeltaTime) override
	{
		TickCount++;
	}

	virtual void EndPlay() override
	{
		bEndPlayCalled = true;
	}
};

class AMockActor : public AActor
{
public:
	UMockComponent* MyComponent = nullptr;

	static UClass* StaticClass()
	{
		static UClass* DummyClass;
		if (!DummyClass)
		{
			DummyClass = new UClass;
			DummyClass->Name = TEXT("AMockActorClass");
			DummyClass->ClassSize = sizeof(AMockActor);
		}

		return DummyClass;
	}

	virtual void PostActorCreated() override
	{
		AActor::PostActorCreated();
		MyComponent = NewObject<UMockComponent>();
		AddComponent(MyComponent);
	}
};

class UMockWorld : public UWorld
{
public:
	static UClass* StaticClass()
	{
		static UClass* DummyClass;
		if (!DummyClass)
		{
			DummyClass = new UClass;
			DummyClass->Name = TEXT("MockWorldClass");
			DummyClass->ClassSize = sizeof(UMockWorld);
		}

		return DummyClass;
	}
};

TEST_F(GameFrameWorkTestFixture, ActorComponentLifecycleAndRouting)
{
	UWorld* TestWorld = NewObject<UMockWorld>();

	AMockActor* SpawnedActor = TestWorld->SpawnActor<AMockActor>();
	EXPECT_NE(SpawnedActor, nullptr) << "Actor spawning failed!";
	EXPECT_EQ(SpawnedActor->GetWorld(), TestWorld) << "Actor's World pointer is incorrect!";

	UMockComponent* MockComp = SpawnedActor->MyComponent;
	EXPECT_NE(MockComp, nullptr) << "Component creation failed!";
	EXPECT_EQ(MockComp->GetOwner(), SpawnedActor) << "Component owner registration failed!";

	EXPECT_FALSE(MockComp->bBeginPlayCalled);
	TestWorld->BeginPlay();
	EXPECT_TRUE(MockComp->bBeginPlayCalled) << "BeginPlay was not routed to the component!";

	EXPECT_EQ(MockComp->TickCount, 0);
	TestWorld->Tick(0.016f);
	TestWorld->Tick(0.016f);
	EXPECT_EQ(MockComp->TickCount, 2) << "Tick was not routed correctly!";

	SpawnedActor->Destroy();
	EXPECT_TRUE(MockComp->bEndPlayCalled) << "EndPlay was not called on Destroy!";

	TestWorld->Tick(0.016f);
	EXPECT_EQ(MockComp->TickCount, 2);
}