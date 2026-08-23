#include "HAL/FMemory.h"
#include "HAL/FMalloc.h"
#include "MISC/AssertionMacros.h"
#include <cstdlib>

class FMallocStd : public FMalloc
{
public:
	virtual void* Malloc(SIZE_T Count, uint32 Alignment) override
	{
		return std::malloc(Count);
	}

	virtual void* Realloc(void* Original, SIZE_T Count, uint32 Alignment) override
	{
		return std::realloc(Original, Count);
	}

	virtual void Free(void* Original)
	{
		std::free(Original);
	}
	virtual const char* GetDescriptiveName() const override { return "Standard CRT Allocator"; }
};

FMalloc* GMalloc = nullptr;
FMalloc* FMemory::GAllocator = nullptr;

void FMemory::SetupMemoryPools()
{
	check(GMalloc == nullptr);
	static FMallocStd StdAllocator;
	GMalloc = &StdAllocator;
	GAllocator = GMalloc;
}

void* FMemory::Malloc(SIZE_T Count, uint32 Alignment)
{
	return GMalloc->Malloc(Count, Alignment);
}

void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
	return GMalloc->Realloc(Original, Count, Alignment);
}

void FMemory::Free(void* Original)
{
	if (Original)
	{
		GMalloc->Free(Original);
	}
}

void* FMemory::Memmove(void* Dest, const void* Src, SIZE_T Count)
{
	return std::memmove(Dest, Src, Count);
}

void* FMemory::Memcpy(void* Dest, const void* Src, SIZE_T Count)
{
	return std::memcpy(Dest, Src, Count);
}

void* FMemory::Memzero(void* Dest, SIZE_T Count)
{
	return std::memset(Dest, 0, Count);
}