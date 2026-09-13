#include "HAL/MallocBinned.h"
#include "TestCheck.h"
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include <cstdio>
#ifdef DAYSTAR_TEST_BINNED_PLATFORM
extern int GBinnedTestFailAfter;
extern std::size_t GBinnedTestPageSize;
std::size_t GetBinnedTestOutstanding();
#endif
static void Empty(const FMallocBinned& A)
{
	auto S = A.GetStats();
	REQUIRE(S.ActiveAllocations == 0 && S.RequestedBytes == 0 && S.BlockCapacityBytes == 0);
	REQUIRE(S.DataRegionBytes == 0 && S.MetadataRegionBytes == 0 && S.SlabCount == 0 && S.LargeRegionCount == 0);
#ifdef DAYSTAR_TEST_BINNED_PLATFORM
	REQUIRE(GetBinnedTestOutstanding() == 0);
#endif
}

int main(int argc, char** argv)
{
#ifdef DAYSTAR_TEST_BINNED_PLATFORM
	if (argc > 1 && std::strcmp(argv[1], "page16k") == 0)GBinnedTestPageSize = 16384;
#endif
	FMallocBinned A;
	if (argc > 1)
	{
		if (std::strcmp(argv[1], "badalign") == 0) { (void)A.TryMalloc(10, 3);return 1; }
		if (std::strcmp(argv[1], "interior") == 0) { auto* P = static_cast<char*>(A.Malloc(32));A.Free(P + 1);return 1; }
		if (std::strcmp(argv[1], "doublefree") == 0) { void* P = A.Malloc(32); (void)A.Malloc(32); A.Free(P); A.Free(P); return 1; }
		if (std::strcmp(argv[1], "live_destroy") == 0) { (void)A.Malloc(32); return 0; }
		if (std::strcmp(argv[1], "foreign") == 0) { int X = 0; A.Free(&X);return 1; }
	}
	REQUIRE(!A.IsInternallyThreadSafe());
	REQUIRE(A.TryMalloc(0) == nullptr);A.Free(nullptr);
	std::size_t N = 123; REQUIRE(!A.GetAllocationSize(nullptr, N) && N == 0);
	REQUIRE(A.TryMalloc((std::numeric_limits<std::size_t>::max)()) == nullptr);
	std::vector<void*>Blocks;
	for (std::size_t Size = 1;Size <= 4096;++Size)
	{
		void* P = A.TryMalloc(Size); REQUIRE(P); REQUIRE(reinterpret_cast<std::uintptr_t>(P) % 16 == 0);
		std::memset(P, static_cast<int>(Size % 251), Size);Blocks.push_back(P);
	}
	for (std::size_t i = 0;i < Blocks.size();++i)
	{
		REQUIRE(A.GetAllocationSize(Blocks[i], N) && N == i + 1);
		auto* P = static_cast<unsigned char*>(Blocks[i]);
		for (std::size_t j = 0;j < N;++j)REQUIRE(P[j] == (i + 1) % 251);
	}
	for (auto P : Blocks)A.Free(P);
	Empty(A);
	void* P = A.Malloc(24); void* Keep = A.Malloc(24);
	auto Calls = A.GetStats().PageAllocationCalls;
	A.Free(P); void* Reused = A.Malloc(24);REQUIRE(Reused == P);
	REQUIRE(A.GetStats().PageAllocationCalls == Calls);
	A.Free(Reused); A.Free(Keep); Empty(A);
	Blocks.clear();
	for (int i = 0;i < 4097;++i)Blocks.push_back(A.Malloc(16));
	REQUIRE(A.GetStats().SlabCount == 2);
	P = Blocks[0]; A.Free(P); Blocks[0] = A.Malloc(16); REQUIRE(Blocks[0] == P);
	for (std::size_t i = 0;i < 4096;++i)A.Free(Blocks[i]);
	REQUIRE(A.GetStats().SlabCount == 1);A.Free(Blocks.back());Empty(A);
	for (auto Align : { 16u,32u,64u,4096u,65536u })
	{
		for (std::size_t Size : {std::size_t(19), std::size_t(4097), std::size_t(65537)})
		{
			P = A.Malloc(Size, Align); REQUIRE(reinterpret_cast<std::uintptr_t>(P) % Align == 0);
			std::memset(P, 0xA5, Size);REQUIRE(A.GetAllocationSize(P, N) && N == Size); A.Free(P);
		}
	}
	Empty(A);
	P = A.Malloc(24); std::memset(P, 0x37, 24);
	void* Q = A.TryRealloc(P, 31); REQUIRE(Q == P);
	Q = A.TryRealloc(P, 100);REQUIRE(Q && Q != P);
	for (int i = 0;i < 24;++i)REQUIRE(static_cast<unsigned char*>(Q)[i] == 0x37);
	P = A.TryRealloc(Q, 200, 65536)REQUIRE(P && reinterpret_cast<std::uintptr_t>(P) % 65536 == 0);
	for (int i = 0;i < 24;++i)REQUIRE(static_cast<unsigned char*>(P)[i] == 0x37);
	REQUIRE(A.TryRealloc(P, 0) == nullptr);Empty(A);
#ifdef DAYSTAR_TEST_BINNED_PLATFORM
	for (int Fail : {0, 1})
	{
		GBinnedTestFailAfter = Fail; REQUIRE(A.TryMalloc(24) == nullptr); Empty(A);
		P = A.Malloc(24); std::memset(P, 0x48, 24);
		auto Before = A.GetStats(); auto Regions = GetBinnedTestOutstanding();
		GBinnedTestFailAfter = Fail; REQUIRE(A.TryRealloc(P, 100000) == nullptr);
		REQUIRE(A.GetAllocationSize(P, N) && N == 24);
		for (int i = 0;i < 24;++i) REQUIRE(static_cast<unsigned char*>(P)[i] == 0x48);
		auto After = A.GetStats();
		REQUIRE(After.RequestedBytes == Before.RequestedBytes && After.DataRegionBytes == Before.DataRegionBytes);
		REQUIRE(After.MetadataRegionBytes == Before.MetadataRegionBytes && GetBinnedTestOutStanding() == Regions);
		A.Free(P); Empty(A);
	}
#endif
	std::puts("Binned contract tests passed");
}