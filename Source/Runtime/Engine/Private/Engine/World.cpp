#include "Engine/World.h"

void UWorld::BeginPlay()
{
	bHasBeginPlay = true;
	for (AActor* Actor : Actors)
	{
		if (Actor && !Actor->bPendingKill)
		{
			Actor->BeginPlay();
		}
	}
}

void UWorld::Tick(float DeltaTime)
{
	for (int32 i = 0;i < Actors.Num();++i)
	{
		AActor* Actor = Actors[i];
		if (Actor)
		{
			if (Actor->bPendingKill)
			{
				Actors[i] = nullptr;
			}
			else
			{
				Actor->Tick(DeltaTime);
			}
		}
	}
}