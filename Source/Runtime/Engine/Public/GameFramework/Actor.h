#pragma once
#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Components/ActorComponent.h"

class ENGINE_API AActor : public UObject
{
public:
	AActor() = default;
	virtual ~AActor() = default;

	virtual void PostActorCreated();
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);
	virtual void Destroy();

	void AddComponent(UActorComponent* Component);

	class UWorld* GetWorld() const { return WorldPrivate; }

private:
	TArray<UActorComponent*> OwnedComponents;
	class UWorld* WorldPrivate = nullptr;

	bool bHasBegunPlay = false;
	bool bPendingKill = false;

	friend class UWorld;
};