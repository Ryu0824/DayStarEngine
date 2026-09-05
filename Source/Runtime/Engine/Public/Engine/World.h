#pragma once
#include "UObject/Object.h"
#include "Containers/Array.h"
#include "GameFrameWork/Actor.h"

class ENGINE_API UWorld : public UObject
{
public:
	UWorld() = default;

	virtual void Tick(float DeltaTime);

	template<typename T>
	T* SpawnActor()
	{
		T* NewActor = NewObject<T>();
		NewActor->WorldPrivate = this;

		NewActor->PostActorCreated();

		Actors.Add(NewActor);

		if (bHasBegunPlay)
		{
			NewActor->BeginPlay();
		}

		return NewActor;
	}

	void BeginPlay();

private:
	TArray<AActor*> Actors;
	bool bHasBeginPlay = false;
};