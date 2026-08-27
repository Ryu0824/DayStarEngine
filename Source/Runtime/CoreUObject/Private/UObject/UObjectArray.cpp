#include "UObject/UObjectArray.h"

FUObjectArray GUObjectArray;

void FUObjectArray::AllocateUObjectIndex(UObject* Object)
{
	for (int32 i = 0;i < ObjArray.Num();++i)
	{
		if (ObjArray.GetData()[i].Object == nullptr)
		{
			ObjArray[i].Object = Object;
			return;
		}
	}

	FUObjectItem NewItem;
	NewItem.Object = Object;
	ObjArray.Add(NewItem);
}

void FUObjectArray::FreeUObjectIndex(UObject* Object)
{
	for (int32 i = 0;i < ObjArray.Num();++i)
	{
		if (ObjArray[i].Object == Object)
		{
			ObjArray[i].Object = nullptr;
			return;
		}
	}
}