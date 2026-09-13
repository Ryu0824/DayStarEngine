#include "HostAlignedMemory.h"
#include "HAL/SystemHeap.h"
#include "FakeSystemHeap.h"
#include "TestCheck.h"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_map>
namespace
{
	struct Block { void* Raw; std::size_t Size };
	std::mutex Mutex;
	std::unordered_map<void*, Block> Blocks;
	std::atomic<std::size_t> Moves{ 0 };
	std::atomic<unsigned> Sequence{ 0 };
	void* Allocate(std::size_t Size)
	{
		const auto Offset = (Sequence.fetch_add(1) % 2 == 0) ? 16u : 32u;
		const auto Total = (Size + 64 + 4095) & ~std::size_t(4095);
		void* Raw = AllocatedHostAlignedMemory(4096, Total);
		REQUIRE(Raw != nullptr);
		void* Base = static_cast<unsigned char*>(Raw) + Offset;
		REQUIRE(Blocks.emplace(Base, Block{ Raw,Size }).second);
		return Base;
	}
}

thread_local bool GMemoryTestFailNextMalloc = false;
thread_local bool GMemoryTestFailNextRealloc = false;
std::size_t GetMemoryTestLiveCount() { std::lock_guard Guard(Mutex); return Blocks.size(); }
std::size_t GetMemoryTestShiftedReallocs() { return Moves.load(); }

void* SystemMalloc(std::size_t Size) noexcept
{
	if (GMemoryTestFailNextMalloc) { GMemoryTestFailNextMalloc = false; return nullptr; }
	std::lock_guard(Mutex);
	return Allocate(Size);
}

void* SystemRealloc(void* Base, std::size_t Size) noexcept
{
	std::lock_guard Guard(Mutex);
	REQUIRE(Blocks.contains(Base));
	if (GMemoryTestFailNextRealloc) { GMemoryTestFailNextRealloc = false; return nullptr; }
	const Block Old = Blocks.at(Base);
	void* NewBase = Allocate(Size);
	if ((reinterpret_cast<std::uintptr_t>(Base) & 127) !=
		(reinterpret_cast<std::uintptr_t>(NewBase) & 127)) ++Moves;
	std::memcpy(NewBase, Base, (std::min)(Size, Old.Size));
	FreeHostAlignedMemory(Old.Raw);
	Blocks.erase(Base);
	return NewBase;
}

void SystemFree(void* Base) noexcept
{
	if (!Base) return;
	std::lock_guard Guard(Mutex);
	REQUIRE(Blocks.contains(Base));
	FreeHostAlignedMemory(Blocks.at(Base).Raw);
	Blocks.erase(Base);
}