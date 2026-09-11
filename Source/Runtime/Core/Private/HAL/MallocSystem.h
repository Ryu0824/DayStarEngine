#pragma once
#include "HAL/FMalloc.h"

class FMallocSystem final : public FMalloc
{
public:
	FMallocSystem() noexcept = default;
	void* TryMalloc(SIZE_T Size, uint32 Alignment = 0) noexcept override;
	void* TryRealloc(void* Original, SIZE_T Size, uint32 Alignment = 0) noexcept override;
	void Free(void* Original) noexcept override;
	bool GetAllocationSize(const void* Original, SIZE_T& Out) const noexcept override;
	bool IsInternallyThreadSafe() const noexcept override { return true; }
	const char* GetDescriptiveName() const noexcept override { return "DayStar CRT-backend system allocator"; }
};