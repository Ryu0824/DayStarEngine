#pragma once

#include "CoreAPI.h"

enum class EPlatformMemoryError : uint8
{
	None,
	InvalidArgument,
	InvalidState,
	SizeOverflow,
	OutOfMemory,
	PlatformFailure
};

struct FPlatformMemoryResult
{
	EPlatformMemoryError Error = EPlatformMemoryError::None;
	uint32 NativeError = 0;

	[[nodiscard]] constexpr bool Succeeded() const noexcept
	{
		return Error == EPlatformMemoryError::None;
	}
};

struct FPlatformMemoryConstants
{
	SIZE_T PageSize = 0;
	SIZE_T AllocationGranularity = 0;
};

struct FPlatformMemoryCapabilities
{
	bool SupportSeparateReserve = false;
	bool SupportPartialActivation = false;
	bool SupportPartialDecativation = false;
	bool SupportProtectionChanges = false;
};

class FPlatformMemory;

class CORE_API FPageRegion final
{
public:
	FPageRegion() noexcept = default;
	~FPageRegion() noexcept;
	FPageRegion(const FPageRegion&) = delete;
	FPageRegion& operator=(const FPageRegion&) = delete;
	FPageRegion(FPageRegion&& Other) noexcept;
	FPageRegion& operator=(FPageRegion&&) = delete;

	[[nodiscard]] bool IsValid() const noexcept { return Base != nullptr; }
	[[nodiscard]] void* GetBase() noexcept { return Base; }
	[[nodiscard]] const void* GetBase() const noexcept { return Base; }
	[[nodiscard]] SIZE_T GetSize() const noexcept { return Size; }
	[[nodiscard]] SIZE_T GetRequestedSize() const noexcept { return RequestedSize; }

private:
	friend class FPlatformMemory;
	void* Base = nullptr;
	SIZE_T Size = 0;
	SIZE_T RequestedSize = 0;
};

class CORE_API FPlatformMemory final
{
public:
	FPlatformMemory() = delete;
	[[nodiscard]] static FPlatformMemoryConstants GetMemoryConstants() noexcept;
	[[nodiscard]] static FPlatformMemoryCapabilities GetMemoryCapabilities() noexcept;

	[[nodiscard]] static FPlatformMemoryResult TryAllocatePages(SIZE_T RequestedSize, FPageRegion& OutRegion) noexcept;
	[[nodisacrd]] static FPlatformMemoryResult ReleasePages(FPageRegion& Region) noexcept;
	[[noreturn]] static void EmergencyTerminate(FPlatformMemoryResult Error) noexcept;
};