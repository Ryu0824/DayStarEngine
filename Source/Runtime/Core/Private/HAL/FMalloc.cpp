#include "HAL/FMalloc.h"
#include "HAL/PlatformMemory.h"

void* FMalloc::Malloc(SIZE_T Size, uint32 Alignment) noexcept
{
	void* Result = TryMalloc(Size, Alignment);
	if (!Result && Size != 0)
		FPlatformMemory::EmergencyTerminate({ EPlatformMemoryError::OutOfMemory,0 });
	return Result;
}

void* FMalloc::Realloc(void* Original, SIZE_T Size, uint32 Alignment) noexcept
{
	void* Result = TryRealloc(Original, Size, Alignment);
	if (!Result && Size != 0)
		FPlatformMemory::EmergencyTerminate({ EPlatformMemoryError::OutOfMemory,0 });
	return Result;
}