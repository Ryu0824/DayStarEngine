#pragma once
#include "CoreTypes.h"
#include "CoreUObjectAPI.h"
#include "Containers/Array.h"

class UObject;

struct FUObjectItem
{
	UObject* Object = nullptr;
	int32 SerialNumber = 0;
};

class COREUOBJECT_API FUObjectArray
{
public:
	void AllocateUObjectIndex(UObject* Object);
	void FreeUObjectIndex(UObject* Object);

	int32 GetObjectCount() const { return ObjArray.Num(); }
	FUObjectItem* GetItem(int32 Index) { return &ObjArray.GetData()[Index]; }

private:
	TArray<FUObjectItem> ObjArray;
};

extern COREUOBJECT_API FUObjectArray GUObjectArray;