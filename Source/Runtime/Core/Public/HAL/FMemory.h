#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"

class FMalloc;

class CORE_API FMemory
{
public:
	static void* Malloc(SIZE_T Count, uint32 Alignment = 8);
	static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 8);
	static void Free(void* Original);
	static void* TryMalloc(SIZE_T Count, uint32 Alignment = 0);
	static void* TryRealloc(void* Original, SIZE_T Count, uint32 Alignment = 0);
	static SIZE_T GetAllocSize(void* Original);

	static void* Memmove(void* Dest, const void* Src, SIZE_T Count);
	static void* Memcpy(void* Dest, const void* Src, SIZE_T Count);
	static void* Memzero(void* Dest, SIZE_T Count);

	static void SetupMemoryPools();

private:
	static FMalloc* GAllocator;
};

extern CORE_API FMalloc* GMalloc;