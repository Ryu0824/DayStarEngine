#pragma once
#include "CoreTypes.h"
#include "CoreUObjectAPI.h"
#include "Containers/UnrealString.h"

class UObject;

class COREUOBJECT_API UField
{
public:
	virtual ~UField() = default;
	FString Name;
};

class COREUOBJECT_API UStruct : public UField
{
public:
	UStruct* SuperStruct = nullptr;
};

class COREUOBJECT_API UClass : public UStruct
{
public:
	using ClassConstructorType = UObject *(*)(void* AllocatedMemory);

	ClassConstructorType ClassConstructor = nullptr;

	int32 ClassSize = 0;
};