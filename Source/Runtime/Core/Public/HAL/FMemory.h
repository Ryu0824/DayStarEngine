#pragma once
#include "CoreAPI.h"

class FMalloc;

class CORE_API FMemory final
{
public:
	FMemory() = delete;
	static void Initialize() noexcept;
	[[nodiscard]] static FMalloc& GetAllocator() noexcept;
	[[nodiscard]] static void* Malloc(SIZE_T Size, uint32 Alignment = 0)noexcept;
	[[nodiscard]] static void* Realloc(void* Original, SIZE_T Size, uint32 Alignment = 0)noexcept;
	[[nodiscard]] static void* TryMalloc(SIZE_T Size, uint32 Alignment = 0)noexcept;
	[[nodiscard]] static void* TryRealloc(void* Original, SIZE_T Size, uint32 Alignment = 0)noexcept;
	static void Free(void* Original) noexcept;
	[[nodiscard]] static bool GetAllocationSize(const void* Original, SIZE_T& Out) noexcept;
	static void* Memcpy(void* Dest, const void* Src, SIZE_T Size)noexcept;
	static void* Memmove(void* Dest, const void* Src, SIZE_T Size)noexcept;
	static void* Memset(void* Dest, uint8 Value, SIZE_T Size)noexcept;
	static void* Memzero(void* Dest, SIZE_T Size)noexcept;
	static int Memcmp(const void* A, const void* B, SIZE_T Size)noexcept;
};