#include "HAL/FMemory.h"
#include "MallocSystem.h"
#include "MISC/AssertionMacros.h"
#include <cstring>
#include <new>

FMalloc& FMemory::GetAllocator() noexcept
{
	alignas(FMallocSystem) static unsigned char Storage[sizeof(FMallocSystem)];
	static FMalloc* const Allocator =
		::new(static_cast<void*>(Storage)) FMallocSystem();

	return *Allocator;
}

void FMemory::Initialize()noexcept { (void)GetAllocator(); }
void* FMemory::Malloc(SIZE_T Size, uint32 Alignment) noexcept { return GetAllocator().Malloc(Size, Alignment); }
void* FMemory::Realloc(void* Original, SIZE_T Size, uint32 Alignment) noexcept { return GetAllocator().Realloc(Original, Size, Alignment); }
void* FMemory::TryMalloc(SIZE_T Size, uint32 Alignment) noexcept { return GetAllocator().TryMalloc(Size, Alignment); }
void* FMemory::TryRealloc(void* Original, SIZE_T Size, uint32 Alignment) noexcept { return GetAllocator().TryRealloc(Original, Size, Alignment); }
void FMemory::Free(void* Original) noexcept { if (Original)GetAllocator().Free(Original); }
bool FMemory::GetAllocationSize(const void* Original, SIZE_T& Out) noexcept
{
	Out = 0;
	return Original ? GetAllocator().GetAllocationSize(Original, Out) : false;
}
void* FMemory::Memcpy(void* Dest, const void* Src, SIZE_T Size) noexcept { return Size ? std::memcpy(Dest, Src, Size) : Dest; }
void* FMemory::Memmove(void* Dest, const void* Src, SIZE_T Size) noexcept { return Size ? std::memmove(Dest, Src, Size) : Dest; }
void* FMemory::Memset(void* Dest, uint8 Value, SIZE_T Size) noexcept { return Size ? std::memset(Dest, Value, Size) : Dest; }
void* FMemory::Memzero(void* Dest, SIZE_T Size)noexcept { return Memset(Dest, 0, Size); }
int FMemory::Memcmp(const void* A, const void* B, SIZE_T Size)noexcept { return Size ? std::memcmp(A, B, Size) : 0; }