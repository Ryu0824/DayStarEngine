#include "HostAlignedMemory.h"
#include "HAL/PlatformMemory.h"
#include "TestCheck.h"
#include <Windows.h>
#include <atomic>
#include <cstring>
#include <limits>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
	std::size_t TestPageSize = 4096;
	constexpr std::size_t TestGranularity = 64 * 1024;
	thread_local DWORD LastError = 0;
	thread_local DWORD FailAllocation = 0;
	thread_local DWORD FailRelease = 0;
	std::atomic<unsigned> AllocationCalls{ 0 }, ReleaseCalls{ 0 }, SystemInfoCalls{ 0 };
	std::mutex MapMutex;
	std::unordered_map<void*, std::size_t> Live;
}

void GetSystemInfo(SYSTEM_INFO* Info)
{
	++SystemInfoCalls;
	Info->dwPageSize = static_cast<DWORD>(TestPageSize);
	Info->dwAllocationGranularity = static_cast<DWORD>(TestGranularity);
}

void* VirtualAlloc(void* Base, std::size_t Size, DWORD Flags, DWORD Protection)
{
	++AllocationCalls;
	REQUIRE(Base == nullptr);
	REQUIRE(Flags == (MEM_RESERVE | MEM_COMMIT));
	REQUIRE(Protection == PAGE_READWRITE);
	REQUIRE(Size != 0 && Size % TestPageSize == 0);
	if (FailAllocation) { LastError = std::exchange(FailAllocation, 0);return 0; }
	const auto HostSize = (Size + TestGranularity - 1) & ~(TestGranularity - 1);
	void* Result = AllocatedHostAlignedMemory(TestGranularity, HostSize);
	REQUIRE(Result != nullptr);
	std::memset(Result, 0, HostSize);
	{ std::lock_guard Guard(MapMutex); REQUIRE(Live.emplace(Result, Size).second); }
	return Result;
}

BOOL VirtualFree(void* Base, std::size_t Size, DWORD Flags)
{
	++ReleaseCalls;
	REQUIRE(Base != nullptr && Size == 0 && Flags == MEM_RELEASE);
	std::lock_guard Guard(MapMutex);
	REQUIRE(Live.contains(Base));
	if (FailRelease) { LastError = std::exchange(FailRelease, 0);return 0; }
	Live.erase(Base);
	FreeHostAlignedMemory(Base);
	return 1;
}

DWORD GetLastError() { return LastError; }
void OutputDebugStringA(const char*) {}
HANDLE GetStdHandle(DWORD) { return nullptr; }
BOOL WriteFile(HANDLE, const void*, DWORD, DWORD*, void*) { return 1; }
HANDLE GetCurrentProcess() { return nullptr; }
BOOL TerminateProcess(HANDLE, unsigned int) { std::_Exit(86); }

int main(int argc, char** argv)
{
	if (argc > 1 && std::strcmp(argv[1], "16384") == 0) TestPageSize = 16384;
	if (argc > 1 && std::strcmp(argv[1], "fatal-release") == 0)
	{
		FPageRegion Region;
		REQUIRE(FPlatformMemory::TryAllocatePages(1, Region).Succeeded());
		FailRelease = 5;
		return 0;
	}
	if (argc > 1 && std::strcmp(argv[1], "bad-system-info") == 0)
	{
		TestPageSize = 3;
		(void)FPlatformMemory::GetMemoryConstants();
		return 1;
	}
	static_assert(!std::is_copy_constructible_v<FPageRegion>);
	static_assert(std::is_nothrow_move_constructible_v<FPageRegion>);
	static_assert(!std::is_move_assignable_v<FPageRegion>);
	std::atomic<bool>Start{ false };
	std::vector<std::thread>Workers;
	for (int i = 0;i < 8;++i)Workers.emplace_back([&] {
		while (!Start.load(std::memory_order_acquire))std::this_thread::yield();
		for (int N = 0;N < 50;++N)
		{
			FPageRegion Region;
			REQUIRE(FPlatformMemory::TryAllocatePages(5000, Region).Succeeded());
			auto* Bytes = static_cast<unsigned char*>(Region.GetBase());
			REQUIRE(Bytes[0] == 0 && Bytes[Region.GetSize() - 1] == 0);
			Bytes[Region.GetSize() - 1] = 73;
		}
		});
	Start.store(true, std::memory_order_release);
	for (auto& Worker : Workers) Worker.join();
	REQUIRE(SystemInfoCalls == 1);
	REQUIRE(Live.empty());
	REQUIRE(FPlatformMemory::GetMemoryConstants().PageSize == TestPageSize);
	const auto Caps = FPlatformMemory::GetMemoryCapabilities();
	REQUIRE(!Caps.SupportSeparateReserve && !Caps.SupportPartialActivation);
	REQUIRE(!Caps.SupportPartialDecativation && !Caps.SupportProtectionChanges);

	FPageRegion Empty;
	const auto BeforeInvalid = AllocationCalls.load();
	REQUIRE(FPlatformMemory::TryAllocatePages(0, Empty).Error == EPlatformMemoryError::InvalidArgument);
	REQUIRE(FPlatformMemory::TryAllocatePages((std::numeric_limits<std::size_t>::max)(), Empty).Error ==
		EPlatformMemoryError::SizeOverflow);
	REQUIRE(AllocationCalls == BeforeInvalid && !Empty.IsValid());
	for (auto Size : { std::size_t(1), TestPageSize - 1,TestPageSize,TestPageSize + 1,TestGranularity + 1 })
	{
		FPageRegion Region;
		REQUIRE(FPlatformMemory::TryAllocatePages(Size, Region).Succeeded());
		REQUIRE(Region.GetSize() >= Size && Region.GetSize() - Size < TestPageSize);
		REQUIRE(Region.GetSize() % TestPageSize == 0);
		void* Base = Region.GetBase();
		const auto BeforeOccupied = AllocationCalls.load();
		REQUIRE(FPlatformMemory::TryAllocatePages(1, Region).Error == EPlatformMemoryError::InvalidState);
		REQUIRE(AllocationCalls == BeforeOccupied && Region.GetBase() == Base);
		FPageRegion Moved(std::move(Region));
		REQUIRE(!Region.IsValid() && Region.GetRequestedSize() == 0);
		REQUIRE(Moved.GetBase() == Base);
		auto* Bytes = static_cast<unsigned char*>(Moved.GetBase());
		for (std::size_t i = 0;i < Moved.GetSize();++i) { REQUIRE(Bytes[i] == 0);Bytes[i] = 42; }
		REQUIRE(FPlatformMemory::ReleasePages(Moved).Succeeded());
		REQUIRE(!Moved.IsValid() && Moved.GetSize() == 0);
		const auto BeforeNoOp = ReleaseCalls.load();
		REQUIRE(FPlatformMemory::ReleasePages(Moved).Succeeded());
		REQUIRE(ReleaseCalls == BeforeNoOp);
	}
	for (DWORD Code : {ERROR_NOT_ENOUGH_MEMORY, ERROR_OUTOFMEMORY, ERROR_COMMITMENT_LIMIT, DWORD(5)})
	{
		FailAllocation = Code;
		const auto Result = FPlatformMemory::TryAllocatePages(1, Empty);
		REQUIRE(Result.NativeError == Code);
		REQUIRE(Result.Error == (Code == 5 ? EPlatformMemoryError::PlatformFailure : EPlatformMemoryError::OutOfMemory));
		REQUIRE(!Empty.IsValid() && Empty.GetSize() == 0 && Empty.GetRequestedSize() == 0);
	}
	{
		FPageRegion Region;
		REQUIRE(FPlatformMemory::TryAllocatePages(3, Region).Succeeded());
		auto* Base = static_cast<unsigned char*>(Region.GetBase());
		Base[0] = 91;
		const auto OriginalSize = Region.GetSize();
		FailRelease = 5;
		const auto Failed = FPlatformMemory::ReleasePages(Region);
		REQUIRE(Failed.Error == EPlatformMemoryError::PlatformFailure && Failed.NativeError == 5);
		REQUIRE(Region.GetBase() == Base && Region.GetSize() == OriginalSize);
		REQUIRE(Region.GetRequestedSize() == 3 && Base[0] == 91);
		REQUIRE(FPlatformMemory::ReleasePages(Region).Succeeded());
		REQUIRE(FPlatformMemory::TryAllocatePages(1, Region).Succeeded());
		std::thread Consumer([Owned = std::move(Region)]()mutable
			{
				REQUIRE(FPlatformMemory::ReleasePages(Owned).Succeeded());
			});
		Consumer.join();
	}
	REQUIRE(Live.empty());
	std::printf("PASS host contract tests : simulated page=%zu; concurrent init, boundaries, ownership, failures\n", TestPageSize);
}