#pragma once
#include "CoreUObjectAPI.h"
#include "UObject/Object.h"
#include "Containers/UnrealString.h"

class COREUOBJECT_API FProperty
{
public:
	FString Name;
	int32 Offset;

	FProperty(const FString& InName, int32 InOffset)
		:Name(InName), Offset(InOffset){}
};

class COREUOBJECT_API UClass : public UObject
{
public:
	FString ClassName;
	UClass* SuperClass;
	TArray<FProperty*> Properties;

	UClass(const FString& InClassName, UClass* InSuperClass)
		:ClassName(InClassName), SuperClass(InSuperClass){ }

	void AddProperty(FProperty* InProperty)
	{
		Properties.Add(InProperty);
	}
};