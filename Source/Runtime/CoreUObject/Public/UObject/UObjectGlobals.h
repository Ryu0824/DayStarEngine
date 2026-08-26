#pragma once
#include "HAL/FMemory.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include <new>
#include "UObjectArray.h"

template<typename T>
T* NewObject()
{
	UClass* TargetClass = T::StaticClass();

	void* ObjMemory = FMemory::Malloc(TargetClass->ClassSize);

	FMemory::Memzero(ObjMemory, TargetClass->ClassSize);
	
	T* NewObj = ::new(ObjMemory) T();

	NewObj->ClassPrivate = TargetClass;
	NewObj->ObjectFlags = RF_NoFlags;

	GUObjectArray.AllocateUObjectIndex(NewObj);

	return NewObj;
}