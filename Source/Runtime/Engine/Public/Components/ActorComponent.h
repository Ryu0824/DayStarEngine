#pragma once
#include "EngineAPI.h"
#include "UObject/Object.h"

class AActor;
class UWorld;

class ENGINE_API UActorComponent : public UObject
{
public:
	UActorComponent() = default;
	virtual ~UActorComponent() = default;

	virtual void InitializeComponent();
	virtual void BeginPlay();
	virtual void TickComponent(float DeltaTime);
	virtual void EndPlay();

	void RegisterComponentWithActor(AActor* InOwner);
	AActor* GetOwner() const { return OwnerPrivate; }

protected:
	AActor* OwnerPrivate = nullptr;
	bool bHasBegunPlay = false;
	bool bIsActive = true;
};