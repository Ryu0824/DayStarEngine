#include "UObject/GarbageCollection.h"
#include "UObject/UObjectArray.h"
#include "UObject/Object.h"
#include "HAL/FMemory.h"

void CollectGarbage()
{
	TArray<UObject*> PendingObjects;

	for (int32 i= 0;i < GUObjectArray.GetObjectCount();++i)
	{
		FUObjectItem* Item = GUObjectArray.GetItem(i);
		if (Item && Item->Object)
		{
			Item->Object->ClearFlags(RF_Reachable);

			if (Item->Object->HasAnyFlags(RF_RootSet))
			{
				Item->Object->SetFlags(RF_Reachable);
				PendingObjects.Add(Item->Object);
			}
		}
	}

	int32 ProcessIndex = 0;
	while (ProcessIndex < PendingObjects.Num())
	{
		UObject* CurrentObj = PendingObjects[ProcessIndex++];

		TArray<UObject*> ReferencedObjects;
		CurrentObj->AddReferencedObjects(ReferencedObjects);

		for (int32 i = 0;i < ReferencedObjects.Num();++i)
		{
			UObject* RefObj = ReferencedObjects[i];
			if (RefObj && !RefObj->HasAnyFlags(RF_Reachable))
			{
				RefObj->SetFlags(RF_Reachable);
				PendingObjects.Add(RefObj);
			}
		}
	}

	for (int32 i = 0;i < GUObjectArray.GetObjectCount();++i)
	{
		FUObjectItem* Item = GUObjectArray.GetItem(i);
		if (Item && Item->Object)
		{
			if (!Item->Object->HasAnyFlags(RF_Reachable))
			{
				UObject* DeadObj = Item->Object;

				GUObjectArray.FreeUObjectIndex(DeadObj);	

				DeadObj->~UObject();
				FMemory::Free(DeadObj);
			}
		}
	}
}