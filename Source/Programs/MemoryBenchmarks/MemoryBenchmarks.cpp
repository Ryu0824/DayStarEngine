#include "HAL/FMemory.h"
#include "HAL/FMalloc.h"
#include "HAL/MallocBinned.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#if defined(_MSC_VER)
#define BENCH_NOINLINE __declspec(noinline)
#else
#define BENCH_NOINLINE __attribute__((noinline))
#endif

namespace
{
	constexpr std::array<std::size_t, 7> RequestSizes{ 16,24,64,256,1024,4096,4097 };
	using FClock = std::chrono::steady_clock;
	static_assert(FClock::is_steady);
	static_assert(sizeof(void*) == 8, "The initial comparison targets x64");
	struct FOptions
	{
		std::size_t Slots = 256;
		std::size_t Cycles = 4;
		std::size_t Repeats = 12;
		std::uint32_t Seed = 12345;
		std::string Output = "memory-benchmark";
		std::string Machine;
	};
	struct FOperation { bool Allocate; std::size_t Slot; std::size_t Size; };
	struct FTrace
	{
		std::string Name;
		std::string Sizes;
		std::vector<FOperation> Operations;
		std::size_t HoldAfter = 0;
		std::size_t Allocations = 0;
		std::size_t HeldCount = 0;
		std::size_t HeldRequested = 0;
	};

	std::uint32_t NextRandom(std::uint32_t& State)
	{
		State ^= State << 13; State ^= State >> 17; State ^= State << 5;
		return State;
	}

	void Shuffle(std::vector<std::size_t>& Values, std::uint32_t& State)
	{
		for (std::size_t i = Values.size(); i > 1; --i)
			std::swap(Values[i - 1], Values[NextRandom(State) % i]);
	}

	void ValidateTrace(FTrace& Trace, std::size_t Slots)
	{
		std::vector<std::size_t> Live(Slots, 0);
		std::size_t Count = 0, Requested = 0, Frees = 0;
		for (std::size_t i = 0;i < Trace.Operations.size();++i)
		{
			const auto& Op = Trace.Operations[i];
			if (Op.Slot >= Slots) throw std::runtime_error("Invalid trace slot");
			if (Op.Allocate)
			{
				if (Live[Op.Slot] || !Op.Size) throw std::runtime_error("Invalid allocation trace");
				Live[Op.Slot] = Op.Size; ++Count; ++Trace.Allocations; Requested += Op.Size;
			}
			else
			{
				if (!Live[Op.Slot]) throw std::runtime_error("Invalid free trace");
				Requested -= Live[Op.Slot];Live[Op.Slot] = 0;--Count;++Frees;
			}
			if (i + 1 == Trace.HoldAfter)
			{
				Trace.HeldCount = Count; Trace.HeldRequested = Requested;
			}
		}
		if (Count || Requested || Frees != Trace.Allocations || !Trace.HeldCount)
			throw std::runtime_error("Unbalanced trace or missing hold checkpoint");
	}

	std::vector<FTrace> BuildTraces(const FOptions& Options)
	{
		std::vector<FTrace> Traces;
		for (const auto Size : RequestSizes)
		{
			FTrace Immediate{ "immediate",std::to_string(Size),{},1 };
			for (std::size_t i = 0;i < Options.Slots * Options.Cycles;++i)
			{
				Immediate.Operations.push_back({ true,0,Size });
				Immediate.Operations.push_back({ false,0,0 });
			}
			Traces.push_back(std::move(Immediate));
			FTrace Batch{ "batch",std::to_string(Size),{},Options.Slots };
			for (std::size_t Round = 0;Round < Options.Cycles;++Round)
			{
				for (std::size_t i = 0;i < Options.Slots;++i)Batch.Operations.push_back({ true,i,Size });
				for (std::size_t i = 0;i < Options.Slots;++i)Batch.Operations.push_back({ false,i,0 });
			}
			Traces.push_back(std::move(Batch));
		}
		std::uint32_t Random = Options.Seed;
		std::vector<std::size_t> Order(Options.Slots);
		for (std::size_t i = 0;i < Order.size();++i)Order[i] = i;
		FTrace Mixed{ "mixed_random_free","mixed",{},Options.Slots };
		for (std::size_t Round = 0;Round < Options.Cycles;++Round)
		{
			for (std::size_t i = 0;i < Options.Slots;++i)
				Mixed.Operations.push_back({ true,i,RequestSizes[NextRandom(Random) % RequestSizes.size()] });
			Shuffle(Order, Random);
			for (const auto i : Order)Mixed.Operations.push_back({ false,i,0 });
		}
		Traces.push_back(std::move(Mixed));
		FTrace Retained{ "retained_half","mixed",{},0 };
		for (std::size_t i = 0;i < Options.Slots;++i)
			Retained.Operations.push_back({ true,i,RequestSizes[NextRandom(Random) % RequestSizes.size()] });
		for (std::size_t Round = 0;Round < Options.Cycles;++Round)
			for (std::size_t i = Options.Slots / 2;i < Options.Slots;++i)
			{
				Retained.Operations.push_back({ false,i,0 });
				Retained.Operations.push_back({ true,i,RequestSizes[NextRandom(Random) % RequestSizes.size()] });
			}
		Retained.HoldAfter = Retained.Operations.size();
		Shuffle(Order, Random);
		for (const auto i : Order) Retained.Operations.push_back({ false,i,0 });
		Traces.push_back(std::move(Retained));
		for (auto& Trace : Traces) ValidateTrace(Trace, Options.Slots);
		return Traces;
	}

	struct FBackend
	{
		const char* Name;
		void* Context;
		void* (*Allocate)(void*, std::size_t);
		void (*Release)(void*, void*);
		bool (*GetSize)(void*, const void*, std::size_t&);
		FMallocBinned* Binned = nullptr;
	};
	BENCH_NOINLINE void* AllocateCrt(void*, std::size_t Size) { return std::malloc(Size); }
	BENCH_NOINLINE void ReleaseCrt(void*, void* Pointer) { std::free(Pointer); }
	BENCH_NOINLINE void* AllocateEngine(void* Context, std::size_t Size)
	{
		return static_cast<FMalloc*>(Context)->TryMalloc(Size, 16);
	}
	BENCH_NOINLINE void ReleaseEngine(void* Context, void* Pointer)
	{
		static_cast<FMalloc*>(Context)->Free(Pointer);
	}

	bool QueryEngine(void* Context, const void* Pointer, std::size_t& Size)
	{
		return static_cast<FMalloc*>(Context)->GetAllocationSize(Pointer, Size);
	}
	struct FSlot { void* Pointer = nullptr; std::size_t Size = 0; };
	struct FScratch
	{
		FBackend& Backend;
		std::vector<FSlot> Slots;
		FScratch(FBackend& InBackend, std::size_t Count) : Backend(InBackend), Slots(Count) {}
		~FScratch()
		{
			for (auto& Slot : Slots)if (Slot.Pointer)Backend.Release(Backend.Context, Slot.Pointer);
		}
		FScratch(const FScratch&) = delete;
		FScratch& operator=(const FScratch&) = delete;
	};

	struct FProbe { FBinnedMemoryStats Held{}; FBinnedMemoryStats After{}; };

	template<bool Verify>
	void Replay(FBackend& Backend, const FTrace& Trace, FScratch& Scratch, FProbe* Probe = nullptr)
	{
		for (std::size_t i = 0;i < Trace.Operations.size();++i)
		{
			const auto& Op = Trace.Operations[i];
			auto& Slot = Scratch.Slots[Op.Slot];
			if (Op.Allocate)
			{
				Slot.Pointer = Backend.Allocate(Backend.Context, Op.Size);
				if (!Slot.Pointer) throw std::runtime_error(std::string(Backend.Name) + " allocation failed; no result accepted");
				Slot.Size = Op.Size;
				if constexpr (Verify)
				{
					if (reinterpret_cast<std::uintptr_t>(Slot.Pointer) % 16 != 0)
						throw std::runtime_error(std::string(Backend.Name) + " does not satisfy 16-byte alignment");
					std::size_t Actual = 0;
					if (Backend.GetSize && (!Backend.GetSize(Backend.Context, Slot.Pointer, Actual) || Actual != Op.Size))
						throw std::runtime_error("Allocation-size mismatch");
				}
				auto* Bytes = static_cast<volatile unsigned char*>(Slot.Pointer);
				Bytes[0] = 0x5A; Bytes[Op.Size - 1] = 0xA5;
			}
			else
			{
				if constexpr (Verify)
				{
					const auto* Bytes = static_cast<const volatile unsigned char*>(Slot.Pointer);
					if (Bytes[0] != 0x5A || Bytes[Slot.Size - 1] != 0xA5)
						throw std::runtime_error("Boundary marker mismatch");
				}
				Backend.Release(Backend.Context, Slot.Pointer);
				Slot.Pointer = nullptr; Slot.Size = 0;
			}
			if constexpr (Verify)
			{
				if (Probe && Backend.Binned && i + 1 == Trace.HoldAfter)
				{
					Probe->Held = Backend.Binned->GetStats();
					if (Probe->Held.ActiveAllocations != Trace.HeldCount || Probe->Held.RequestedBytes != Trace.HeldRequested)
						throw std::runtime_error("Binned hold statistics disagree with trace");
				}
			}
		}
	}

	void CheckEmpty(const FBinnedMemoryStats& Stats)
	{
		if (Stats.ActiveAllocations || Stats.RequestedBytes || Stats.BlockCapacityBytes ||
			Stats.DataRegionBytes || Stats.MetadataRegionBytes || Stats.SlabCount || Stats.LargeRegionCount)
			throw std::runtime_error("Binned retained live blocks or regions after trace cleanup");
	}

	struct FSample
	{
		std::size_t Round = 0;
		std::size_t Position = 0;
		double ElapsedNs = 0;
		std::size_t PageCalls = 0;
		FBinnedMemoryStats After{};
	};

	FSample Measure(FBackend& Backend, const FTrace& Trace, const FOptions& Options)
	{
		FScratch Scratch(Backend, Options.Slots);
		const auto Before = Backend.Binned ? Backend.Binned->GetStats().PageAllocationCalls : 0;
		const auto Begin = FClock::now();
		Replay<false>(Backend, Trace, Scratch);
		const auto End = FClock::now();
		FSample Sample;
		Sample.ElapsedNs = std::chrono::duration<double, std::nano>(End - Begin).count();
		if (Sample.ElapsedNs <= 0) throw std::runtime_error("Interval below clock resolution; increase slots/cycles");
		if (Backend.Binned)
		{
			Sample.After = Backend.Binned->GetStats();
			Sample.PageCalls = Sample.After.PageAllocationCalls - Before;
			CheckEmpty(Sample.After);
		}
		return Sample;
	}

	std::string Quote(std::string_view Text)
	{
		std::string Result = "\"";
		for (const char C : Text) { if (C == '"')Result += '"';Result += C; }
		return Result + '"';
	}

	std::string CompilerName()
	{
#if defined(_MSC_VER)
		return "MSVC" + std::to_string(_MSC_FULL_VER);
#elif defined(__clang__)
		return "Clang" __clang_version__;
#else
		return "GCC" __VERSION__;
#endif
	}

	std::string PlatformName()
	{
#if defined(_WIN32)
		return "Windows x64";
#else
		return "Non-Windows host (not a native engine benchmark)";
#endif
	}

	std::size_t ParseNumber(std::string_view Text, std::size_t Min, std::size_t Max)
	{
		std::size_t Value = 0;
		const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
		if (Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size() || Value<Min || Value>Max)
			throw std::runtime_error("Numeric argument outside supported range");

		return Value;
	}

	FOptions ParseOptions(int Argc, char** Argv)
	{
		FOptions Options;
		for (int i = 1;i < Argc;++i)
		{
			const std::string_view Key = Argv[i];
			if (i + 1 == Argc) throw std::runtime_error("Option requires a value");
			const std::string_view Value = Argv[++i];
			if (Key == "--slots") Options.Slots = ParseNumber(Value, 2, 4096);
			else if (Key == "--cycles") Options.Cycles = ParseNumber(Value, 1, 64);
			else if (Key == "--repeats") Options.Repeats = ParseNumber(Value, 3, 99);
			else if (Key == "--seed")Options.Seed = static_cast<std::uint32_t>(ParseNumber(Value, 1, UINT32_MAX));
			else if (Key == "--output")Options.Output = Value;
			else if (Key == "--machine")Options.Machine = Value;
			else throw std::runtime_error("Unknown option: " + std::string(Key));
		}
		if (Options.Output.empty()) throw std::runtime_error("Output prefix cannot be empty");
		if (Options.Machine.empty())
		{
			const char* Cpu = std::getenv("PROCESSOR_IDENTIFIER");
			Options.Machine = Cpu ? Cpu : "unspecified (use --machine)";
		}
		return Options;
	}

	void WriteStats(std::ostream& Stream, const FBinnedMemoryStats& Stats)
	{
		Stream << Stats.ActiveAllocations << ',' << Stats.RequestedBytes << ',' << Stats.BlockCapacityBytes << ','
			<< Stats.DataRegionBytes << ',' << Stats.MetadataRegionBytes << ',' << Stats.SlabCount << ',' << Stats.LargeRegionCount;
	}
}

int main(int Argc, char** Argv)
{
	try
	{
		if (Argc == 2 && std::string_view(Argv[1]) == "--help")
		{
			std::cout << "MemoryBenchmarks [--slots 2..4096] [--cycles 1..64] [--repeats 3..99]\n"
				" [--seed 1..4294967295] [--output path/prefix] [--machine label]\n"
				"Default: 256 slots, 4 cycles, 12 repeats, seed 12345. Release only.\n";

			return 0;
		}
#ifndef NDEBUG
		throw std::runtime_error("Use a Release build without a debugger for measurement");
#endif
		const auto Options = ParseOptions(Argc, Argv);
		const auto Traces = BuildTraces(Options);
		FMemory::Initialize();
		FMalloc& System = FMemory::GetAllocator();
		if (std::string_view(System.GetDescriptiveName()) != "DayStar CRT-backend system allocator")
			throw std::runtime_error("FMemory backend changed; update benchmark labels/adapters before comparing");
		FMallocBinned Binned;
		std::array<FBackend, 3> Backends{ {
				{"System",&System,AllocateEngine,ReleaseEngine,QueryEngine,nullptr},
				{"CRT",nullptr,AllocateCrt,ReleaseCrt,nullptr,nullptr},
				{"Binned",static_cast<FMalloc*>(&Binned),AllocateEngine,ReleaseEngine,QueryEngine,&Binned}
			} };
		const auto RunId = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		const std::filesystem::path SamplesPath = Options.Output + ".samples.csv";
		const std::filesystem::path SummaryPath = Options.Output + ".summary.csv";
		if (std::filesystem::exists(SamplesPath) || std::filesystem::exists(SummaryPath))
			throw std::runtime_error("Output already exists; choose a new --output prefix");
		if (!SamplesPath.parent_path().empty()) std::filesystem::create_directories(SamplesPath.parent_path());
		std::ofstream Samples(SamplesPath), Summary(SummaryPath);
		if (!Samples || !Summary)throw std::runtime_error("Cannot create output CSV files");
		Samples.imbue(std::locale::classic());Summary.imbue(std::locale::classic());
		Samples << std::setprecision(17);Summary << std::setprecision(17);
		const std::string EnvironmentHeader = "run_id,platform,compiler,machine,logical_threads,seed,slots,cycles,repeats,warmups,alignment,touch_policy,";
		Samples << EnvironmentHeader << "case,sizes,backend,round,order_position,allocation_pairs,operation_count,elapsed_ns,ns_per_pair,hold_checkpoint_op,held_count_trace,held_requested_trace,stats_available,probe_held_active,probe_held_requested,probe_held_capacity,probe_held_data,probe_held_metadata,probe_held_slabs,probe_held_large,after_active,after_requested,after_capacity,after_data,after_metadata,after_slabs,after_large,timed_page_allocation_calls\n";
		Summary << EnvironmentHeader << "case,sizes,backend,allocation_pairs,min_ns_per_pair,median_ns_per_pair,max_ns_per_pair\n";
		const auto WriteEnvironment = [&](std::ostream& Stream)
			{
				Stream << RunId << ',' << Quote(PlatformName()) << ',' << Quote(CompilerName()) << ','
					<< Quote(Options.Machine) << ',' << std::thread::hardware_concurrency() << ','
					<< Options.Seed << ',' << Options.Slots << ',' << Options.Cycles << ',' << Options.Repeats
					<< ",1,16,volatile_first_last_byte,";
			};
		std::cout << PlatformName() << "; " << CompilerName() << "; machine=" << Options.Machine << '\n'
			<< "Single thread; whole-trace timing includes dispatch/bookkeeping and boundary writes.\n"
			<< "Hold statistics come from a separate untimed replay; CRT/System internal usage is unavailable.\n";
		constexpr std::array<std::array<std::size_t, 3>, 6>Orders{ {
			{{0,1,2}},{{1,2,0}},{{2,0,1}},{{2,1,0}},{{1,0,2}},{{0,2,1}}
			} };
		for (std::size_t Case = 0;Case < Traces.size();++Case)
		{
			const auto& Trace = Traces[Case];
			std::array<FProbe, 3>Probes{};
			std::array<std::vector<FSample>, 3>Results;
			for (auto& Rows : Results)Rows.reserve(Options.Repeats);

			for (const auto Index : Orders[Case % Orders.size()])
			{
				auto& Backend = Backends[Index];
				FScratch Scratch(Backend, Options.Slots);
				Replay<true>(Backend, Trace, Scratch, &Probes[Index]);
				if (Backend.Binned)
				{
					Probes[Index].After = Backend.Binned->GetStats();
					CheckEmpty(Probes[Index].After);
				}
				(void)Measure(Backend, Trace, Options);
			}

			for (std::size_t Round = 0;Round < Options.Repeats;++Round)
			{
				const auto& Order = Orders[(Case + Round) % Orders.size()];
				for (std::size_t Position = 0; Position < Order.size();++Position)
				{
					const auto Index = Order[Position];
					auto Sample = Measure(Backends[Index], Trace, Options);
					Sample.Round = Round + 1;Sample.Position = Position + 1;
					Results[Index].push_back(Sample);
				}
			}

			for (std::size_t Index = 0;Index < Backends.size();++Index)
			{
				std::vector<double> PairTimes;
				PairTimes.reserve(Options.Repeats);
				for (const auto& Sample : Results[Index])
				{
					const double PairNs = Sample.ElapsedNs / static_cast<double>(Trace.Allocations);
					PairTimes.push_back(PairNs);
					WriteEnvironment(Samples);
					Samples << Trace.Name << ',' << Trace.Sizes << ',' << Backends[Index].Name << ','
						<< Sample.Round << ',' << Sample.Position << ',' << Trace.Allocations << ','
						<< Trace.Operations.size() << ',' << Sample.ElapsedNs << ',' << PairNs << ','
						<< Trace.HoldAfter << ',' << Trace.HeldCount << ',' << Trace.HeldRequested << ',';
					if (Backends[Index].Binned)
					{
						Samples << "1,";WriteStats(Samples, Probes[Index].Held);Samples << ',';
						WriteStats(Samples, Sample.After); Samples << ',' << Sample.PageCalls;
					}
					else
					{
						Samples << '0';
						for (int Column = 0;Column < 15;++Column)Samples << ',';
					}
					Samples << '\n';
				}
				std::sort(PairTimes.begin(), PairTimes.end());
				const auto Middle = PairTimes.size() / 2;
				const double Median = PairTimes.size() % 2 ? PairTimes[Middle] : (PairTimes[Middle - 1] + PairTimes[Middle]) / 2;
				WriteEnvironment(Summary);
				Summary << Trace.Name << ',' << Trace.Sizes << ',' << Backends[Index].Name << ',' << Trace.Allocations
					<< ',' << PairTimes.front() << ',' << Median << ',' << PairTimes.back() << '\n';
				std::cout << Trace.Name << '/' << Trace.Sizes << ' ' << Backends[Index].Name << ": "
					<< std::fixed << std::setprecision(2) << "median=" << Median << " ns/pair, min="
					<< PairTimes.front() << ", max=" << PairTimes.back() << '\n';
			}
			Samples.flush(); Summary.flush();
			if (!Samples || !Summary) throw std::runtime_error("CSV write failed; results incomplete");
		}
		Samples.close(); Summary.close();
		if (Samples.fail() || Summary.fail()) throw std::runtime_error("CSV close failed; results incomplete");
		std::cout << "COMPLETE: " << SamplesPath << " and " << SummaryPath << '\n';
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Benchmark aborted: " << Error.what() << "\n Any CSV from this run is incomplete; do not compare it. \n";
		return 1;
	}
}