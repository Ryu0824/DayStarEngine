#pragma once
#include "CoreTypes.h"
#include "CoreUObjectAPI.h"
#include "UObject/ObjectMacros.h"

class UClass;

enum EObjectFlags : uint32
{
	RF_NoFlags = 0,
	RF_Public = 1 << 0,
	RF_Marked = 1 << 1,
	RF_PendingKill = 1 << 2,
};

class COREUOBJECT_API UObject
{
public:
	UObject();
	virtual ~UObject();

	UClass* GetClass() const { return ClassPrivate; }

	void* operator new(SIZE_T Size) = delete;
	void operator delete(void* Ptr) = delete;

private:
	UClass* ClassPrivate;
	uint32 ObjectFlags;

	template<typename T> friend T* NewObject();
};