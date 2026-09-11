#include "HAL/PlatformMemory.h"

#if !defined(_WIN32)
# error This implementation REQUIRES WINDOWS!!. No LINUX Backend is provided yet.
#endif

#if defined(_WIN32) && !defined(_M_X64) && !defined(__X86_64__)
# error This milestone targets Windows x64 Only
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif

#include <Windows.h>
#include <cstdlib>
#include <limits>

static_assert(sizeof(SIZE_T) == 8, "64-bit target required");

FPlatformMemoryResult FromNativeFailure(DWORD Code) noexcept
{
	switch (Code)
	{
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
	case ERROR_COMMITMENT_LIMIT:
		return { EPlatformMemoryError::OutOfMemory,Code };
	default:
		return { EPlatformMemoryError::PlatformFailure,Code };
	}
}

bool IsPowerOfTwo(SIZE_T Value) noexcept
{
	return Value != 0 && (Value & (Value - 1)) == 0;
}

char* AppendHex(char* Output, uint32 Value) noexcept
{
	constexpr char Digits[] = "0123456789ABCDEF";
	for (int Shift = 28; Shift >= 0;Shift -= 4)
		*Output++ = Digits[(Value >> Shift) & 0xFu];
	return Output;
}

FPlatformMemoryConstants FPlatformMemory::GetMemoryConstants() noexcept
{
	static const FPlatformMemoryConstants Constants = []() noexcept
		{
			SYSTEM_INFO Info{};
			::GetSystemInfo(&Info);
			const FPlatformMemoryConstants Result{
				static_cast<SIZE_T>(Info.dwPageSize),
				static_cast<SIZE_T>(Info.dwAllocationGranularity)};

			if (!IsPowerOfTwo(Result.PageSize) ||
				!IsPowerOfTwo(Result.AllocationGranularity) ||
				Result.AllocationGranularity < Result.PageSize)
			{
				EmergencyTerminate({ EPlatformMemoryError::PlatformFailure,0 });
			}
			return Result;
		}();
	return Constants;
}

FPlatformMemoryCapabilities FPlatformMemory::GetMemoryCapabilities() noexcept
{
	return {};
}

FPlatformMemoryResult FPlatformMemory::TryAllocatePages(SIZE_T RequestedSize, FPageRegion& OutRegion) noexcept
{
	if (OutRegion.IsValid())
		return { EPlatformMemoryError::InvalidState,0 };
	if (RequestedSize == 0)
		return { EPlatformMemoryError::InvalidArgument,0 };

	const auto Constants = GetMemoryConstants();
	constexpr auto Maximum = (std::numeric_limits<SIZE_T>::max)();
	const auto Padding = Constants.PageSize - 1;
	if (RequestedSize > Maximum - Padding)
		return { EPlatformMemoryError::SizeOverflow,0 };
	const auto RoundedSize = (RequestedSize + Padding) & ~Padding;

	void* Base = ::VirtualAlloc(nullptr, RoundedSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	if (!Base)
	{
		const DWORD Code = ::GetLastError();
		return FromNativeFailure(Code);
	}

	OutRegion.Base = Base;
	OutRegion.Size = RoundedSize;
	OutRegion.RequestedSize = RequestedSize;
	return {};
}

FPlatformMemoryResult FPlatformMemory::ReleasePages(FPageRegion& Region) noexcept
{
	if (!Region.IsValid())
		return{};

	if (!::VirtualFree(Region.Base, 0, MEM_RELEASE))
	{
		const DWORD Code = ::GetLastError();
		return FromNativeFailure(Code);
	}

	Region.Base = nullptr;
	Region.Size = 0;
	Region.RequestedSize = 0;
	return {};
}

[[noreturn]] void FPlatformMemory::EmergencyTerminate(FPlatformMemoryResult Error) noexcept
{
	char Message[96]{};
	char* Cursor = Message;
	constexpr char Prefix[] = "DayStar platform memory fatal : error 0x";
	for (SIZE_T Index = 0;Index < sizeof(Prefix) - 1;++Index)
		*Cursor++ = Prefix[Index];
	Cursor = AppendHex(Cursor, static_cast<uint32>(Error.Error));
	constexpr char NativePrefix[] = " native=0x";
	for (SIZE_T Index = 0;Index < sizeof(NativePrefix) - 1;++Index)
		*Cursor++ = NativePrefix[Index];
	Cursor = AppendHex(Cursor, Error.NativeError);
	*Cursor++ = '\n';
	*Cursor = '\0';

	::OutputDebugStringA(Message);
	const HANDLE Stderr = ::GetStdHandle(STD_ERROR_HANDLE);
	if (Stderr != nullptr && Stderr != INVALID_HANDLE_VALUE)
	{
		DWORD Written = 0;
		(void)::WriteFile(Stderr, Message, static_cast<DWORD>(Cursor - Message),
			&Written, nullptr);
	}
	(void)::TerminateProcess(::GetCurrentProcess(), 0xE0000001u);
	std::abort();
}