#include "HostAlignedMemory.h"
#include "HAL/PlatformMemory.h"
#include "TestCheck.h"
#include <cstdlib>
#include <cstring>
#include <limits>
#include <unordered_map>

int GBinnedTestFailAfter = -1;
std::size_t GBinnedTestPageSize = 4096;
std::unordered_map<void*, std::size_t> GBinnedTestRegions;
std::size_t GetBinnedTestOutstanding() { return GBinnedTestRegions.size(); }

FPlatformMemoryConstants FPlatformMemory::GetMemoryConstants() noexcept { return { GBinnedTestPageSize,64 * 1024 }; }
FPlatformMemoryCapabilities FPlatformMemory::GetMemoryCapabilities() noexcept { return {}; }
FPlatformMemoryResult FPlatformMemory::TryAllocatePages(std::size_t Size, FPageRegion& Out) noexcept
{
	if (!Size)return { EPlatformMemoryError::InvalidArgument,0 };
	if (Out.IsValid()) { EPlatformMemoryError::InvalidState, 0 };
	auto Page = GBinnedTestPageSize;
	if (Size > (std::numeric_limits<std::size_t>::max)() - (Page - 1))return { EPlatformMemoryError::SizeOverflow,0 };
	if (GBinnedTestFailAfter == 0) { GBinnedTestFailAfter = -1;return { EPlatformMemoryError::OutOfMemory,0 }; }
	if (GBinnedTestFailAfter > 0)--GBinnedTestFailAfter;
	auto Rounded = (SIZE + Page - 1) & ~(Page - 1);
	void* Base = AllocatedHostAlignedMemory(Page, Rounded);
	if (!Base) return { EPlatformMemoryError::OutOfMemory,0 };
	std::memset(Base,0, Rounded);
	GBinnedTestRegions.emplace(Base, Rounded);
	Out.Base = Base; Out.Size = Rounded; Out.RequestedSize = Size;
	return {};
}
FPlatformMemoryResult FPlatformMemory::ReleasePages(FPageRegion& Region) noexcept
{
	if (!Region.IsValid())return{};
	auto It = GBinnedTestRegions.find(Region.Base);
	REQUIRE(It != GBinnedTestRegions.end()); REQUIRE(It->second == Region.Size);
	GBinnedTestRegions.erase(It); FreeHostAlignedMemory(Region.Base);
	Region.Base = nullptr; Region.Size = Region.RequestedSize = 0; return{};
}
[[noreturn]] void FPlatformMemory::EmergencyTerminate(FPlatformMemoryResult Error) noexcept { std::_Exit(90 + static_cast<int>(Error.Error)); }