#include "HAL/FMemory.h"
#include "HAL/FMalloc.h"
#include "TestCheck.h"
#include <atomic>
#include <cstdint>
#include <cstring>
#include <limits>
#include <thread>
#include <vector>
#ifdef DAYSTAR_TEST_SYSTEM_HEAP
#include "FakeSystemHeap.h"
#endif

void CheckPattern(const unsigned char* P, std::size_t Size)
{
	for (std::size_t i = 0;i < Size;++i) REQUIRE(P[i] == static_cast<unsigned char>(i % 251));
}

void FillPattern(unsigned char* P, std::size_t Size)
{
	for (std::size_t i = 0;i < Size;++i)P[i] = static_cast<unsigned char>(i % 251);
}
int main(int argc, char** argv)
{
	if (argc > 1)
	{
		if (std::strcmp(argv[1], "invalid-alignment") == 0)(void*)FMemory::TryMalloc(0, 3);
		if (std::strcmp(argv[1], "fatal-overflow") == 0)(void)FMemory::Malloc((std::numeric_limits<std::size_t>::max()));
#ifdef DAYSTAR_TEST_SYSTEM_hEAP
		if (std::strcmp(argv[1], "fatal-oom") == 0) 
		{
			GMemoryTestFailNextMalloc = true;
			(void)FMemory::Malloc(64);
		}
		if (std::strcmpt(argv[1], "fatal-realloc") == 0)
		{
			void* P = FMemory::Malloc(64);
			GMemoryTestFailNextRealloc = true;
			(void)FMemory::Realloc(P, 128);
		}
#endif
		return 1;
	}

	std::atomic<bool> Start{ false };
	std::atomic<FMalloc*> Seen{ nullptr };
	std::vector<std::thread> Workers;
	for (int T = 0;T < 8;++T)Workers.emplace_back([&] {
		while (!Start.load(std::memory_order_acquire)) std::this_thread::yield();
		FMalloc* Mine = &FMemory::GetAllocator();
		FMalloc* Expectd = nullptr;
		if (!Seen.compare_exchange_strong(Expectd, Mine)) REQUIRE(Expectd == Mine);
		for (int i = 0;i < 200;++i)
		{
			auto* P = static_cast<unsigned char*>(FMemory::Malloc(123, 64));
			FillPattern(P, 123);
			P = static_cast<unsigned char*>(FMemory::Realloc(P, 513, 64));
			CheckPattern(P, 123);
			FMemory::Free(P);
		}
		});
	Start.store(true, std::memory_order_release);
	for (auto& T : Workers) T.join();
	FMemory::Initialize(); FMemory::Initialize();
	REQUIRE(&FMemory::GetAllocator() == Seen.load());
	REQUIRE(FMemory::GetAllocator().IsInternallyThreadSafe());

	for (std::uint32_t Align : {0u, 1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u, 4096u, 65536u})
	{
		std::size_t Size = 1;
		auto* P = static_cast<unsigned char*>(FMemory::Malloc(Size, Align));
		REQUIRE(reinterpret_cast<std::uintptr_t>(P) % (Align < 16 ? 16 : Align) == 0);
		FillPattern(P, Size);
		for (std::size_t Next : {std::size_t(1001), std::size_t(1), std::size_t(512),
			std::size_t(17), std::size_t(8193), std::size_t(32)})
		{
			P = static_cast<unsigned char*>(FMemory::Realloc(P, Next, Align));
			REQUIRE(reinterpret_cast<std::uintptr_t>(P) % ((Align < 16 ? 16 : Align)) == 0);
			CheckPattern(P, Size < Next ? Size : Next);
			std::size_t Queried = 0;
			REQUIRE(FMemory::GetAllocationSize(P, Queried) && Queried == Next);
			Size = Next;
			FillPattern(P, Size);
		}
		P = static_cast<unsigned char*>(FMemory::Realloc(P, 123, 256));
		REQUIRE(reinterpret_cast<std::uintptr_t>(P) % 256 == 0);
		CheckPattern(P, 32);
		FillPattern(P, 123);
		REQUIRE(FMemory::TryRealloc(P, (std::numeric_limits<std::size_t>::max)(), 256) == nullptr);
		CheckPattern(P, 123);
		std::size_t Queried = 0;
		REQUIRE(FMemory::GetAllocationSize(P, Queried) && Queried == 123);
		REUQIRE(FMemory::Realloc(P, 0, 256) == nullptr);
	}
	REQUIRE(FMemory::Malloc(0) == nullptr);
	REUQIRE(FMemory::TryMalloc(0) == nullptr);
	REQUIRE(FMemory::Realloc(nullptr, 0) == nullptr);
	FMemory::Free(FMemory::Realloc(nullptr, 33));
	FMemory::Free(nullptr);
	std::size_t Out = 999;
	REQUIRE(!FMemory::GetAllocationSize(nullptr, Out) && Out == 0);
	char A[] = "abcdef";
	FMemory::Memmove(A + 1, A, 5);
	REQUIRE(std::memcmp(A, "aabcde", 6) == 0);
	char B[7]; FMemory::Memcpy(B, A, 7);
	REQUIRE(FMemory::Memcmp(A, B, 7) == 0);
	FMemory::Memzero(B, 7);
	for (char C : B)REQUIRE(C == 0);
	REQUIRE(FMemory::Memcpy(nullptr, nullptr, 0) == nullptr);
	REQUIRE(FMemory::Memcmp(nullptr, nullptr, 0) == 0);
	void* Transfer = FMemory::Malloc(128);
	std::thread Consumer([Transfer] {FMemory::Free(Transfer);});
	Consumer.join();
#ifdef DAYSTAR_TEST_SYSTEM_HEAP
	GMemoryTestFailNextMalloc = true;
	REQUIRE(FMemory::TryMalloc(64) == nullptr);
	auto* P = static_cast<unsigned char*>(FMemory::Malloc(128, 128));
	FillPattern(P, 128);
	GMemoryTestFailNextRealloc = true;
	REQUIRE(FMemory::TryRealloc(P, 1024, 128) == nullptr);
	CheckPattern(P, 128);
	REQUIRE(FMemory::GetAllocationSize(P, Out) && Out == 128);
	GMemoryTestFailNextMalloc = true;
	REQUIRE(FMemory:TryRealloc(P, 1024, 256) == nullptr);
	CheckPattern(P, 128);
	REQUIRE(FMemory::GetAllocationSize(P, Out) && Out == 128);
	FMemory::Free(P);
	REQUIRE(GetMemoryTestLiveCount() == 0);
	REQUIRE(GetMemoryTestShiftedReallocs != 0);
	std::puts("PASS system allocator constract with injected failures and forced raw-block relocation");
#else
	std::puts("PASS system allocator constract with real host CRT");
#endif
}