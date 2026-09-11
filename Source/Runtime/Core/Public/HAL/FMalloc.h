#pragma once
#include "CoreAPI.h"

class CORE_API FMalloc
{
public:
	[[nodiscrad]] void* Malloc(SIZE_T Size, uint32 Alignment = 0)noexcept;
	[[nodisacrd]] void* Realloc(void* Original, SIZE_T Size, uint32 Alignment = 0)noexcept;

	[[nodiscard]] virtual void* TryMalloc(SIZE_T Size, uint32 Alignment = 0)noexcept = 0;
	[[nodisacrd]] virtual void* TryRealloc(void* Original, SIZE_T Size, uint32 Alignment = 0) noexcept = 0;

	virtual void Free(void* Original) noexcept = 0;
	[[nodisacrd]] virtual bool GetAllocationSize(const void* Original, SIZE_T& Out) const noexcept = 0;
	[[nodisacrd]] virtual bool IsInternallyThreadSafe() const noexcept = 0;
	[[nodisacrd]] virtual const char* GetDescriptiveName() const noexcept = 0;

protected:
	virtual ~FMalloc() = default;
};