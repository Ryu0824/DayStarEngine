#include "Components/ActorComponent.h"

void UActorComponent::InitializeComponent(){}

void UActorComponent::BeginPlay()
{
	bHasBegunPlay = true;
}

void UActorComponent::TickComponent(float DeltaTime){}

void UActorComponent::EndPlay(){}

void UActorComponent::RegisterComponentWithActor(AActor* InOwner)
{
	OwnerPrivate = InOwner;
}