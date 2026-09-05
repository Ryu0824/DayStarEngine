#include "GameFramework/Actor.h"

void AActor::PostActorCreated(){}

void AActor::BeginPlay()
{
	bHasBegunPlay = true;

	for (UActorComponent* Comp : OwnedComponents)
	{
		if (Comp) { Comp->BeginPlay(); }
	}
}

void AActor::Tick(float DeltaTime)
{
	if (bPendingKill) return;

	for (UActorComponent* Comp : OwnedComponents)
	{
		if (Comp) { Comp->TickComponent(DeltaTime); }
	}
}

void AActor::Destroy()
{
	bPendingKill = true;
	for (UActorComponent* Comp : OwnedComponents)
	{
		if (Comp) { Comp->EndPlay(); }
	}
}

void AActor::AddComponent(UActorComponent* Component)
{
	if (Component)
	{
		Component->RegisterComponentWithActor(this);
		OwnedComponents.Add(Component);
	}
}