#include "HAL/MallocBinned.h"
#include "HAL/PlatformMemory.h"
#include <cstring>
#include <limits>
#include <new>
#include <utility>

constexpr SIZE_T Sizes[] = {
	16,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,
	384,512,768,1024,1536,2048,3072,4096 };
constexpr SIZE_T SmallLimit = 4096;
constexpr SIZE_T SlabTarget = 64 * 1024;
constexpr SIZE_T LargeClass = 24;
constexpr uint32 NoBlock = (std::numeric_limits<uint32>::max)();

[[noreturn]] void InvalidUse() noexcept
{
	FPlatformMemory::EmergencyTerminate({ EPlatformMemoryError::InvalidArgument,0 });
}

// A function that checks whether the address alignment value is a power of 2.
SIZE_T Normalize(uint32 Alignment) noexcept
{
	SIZE_T Result = Alignment == 0 ? 16 : Alignment;
	if ((Result & (Result - 1)) != 0) InvalidUse();
	return Result < 16 ? 16 : Result;
}

// An addition function that checks whether a calculation exceeds the range of virtual address values ​​representable by the system.
bool Add(SIZE_T A, SIZE_T B, SIZE_T& Out) noexcept
{
	// The reason it is not expressed as A+B is that A+B might exceed the range of values ​​representable by the corresponding variable type.
	if (A > (std::numeric_limits<SIZE_T>::max)() - B)return false;
	Out = A + B;
	return true;
}


bool AppendArray(SIZE_T Count, SIZE_T ItemSize, SIZE_T Alignment, SIZE_T& Total, SIZE_T& Offset) noexcept
{
	SIZE_T Aligned = 0;

	// Ensure that the total data value includes the maximum padding bit value required for the alignment.
	if (!Add(Total, Alignment - 1, Aligned))return false;

	// If the values ​​are already sorted, reset them.
	Aligned &= ~(Alignment - 1);
	constexpr auto Maximum = (std::numeric_limits<SIZE_T>::max)();
	if (Count > (Maximum - Aligned) / ItemSize)return false;
	Offset = Aligned;
	Total = Aligned + Count * ItemSize;
	return true;
}

struct FMallocBinned::FBlockRecord
{
	uint32 Requested = 0;
	uint32 Next = NoBlock;
};

struct FMallocBinned::FPageEntry
{
	std::uintptr_t Key = 0;
	FSpan* Span = nullptr;
	FPageEntry* Next = nullptr;
};

struct FMallocBinned::FSpan
{
	FPageRegion Data;
	FPageRegion Metadata;
	SIZE_T ClassIndex;
	unsigned char* UserBase = nullptr;
	SIZE_T Capacity = 0;
	SIZE_T Requested = 0;
	SIZE_T Count = 0;
	SIZE_T Used = 0;
	uint32 FreeHead = NoBlock;
	FBlockRecord* Records = nullptr;
	FPageEntry* Entries = nullptr;
	SIZE_T EntryCount = 0;
	FSpan* AvailablePrev = nullptr;
	FSpan* AvailableNext = nullptr;
	FSpan* AllPrev = nullptr;
	FSpan* AllNext = nullptr;
	FSpan(FPageRegion&& InData, FPageRegion&& InMeta, SIZE_T InClass) noexcept
		:Data(std::move(InData)), Metadata(std::move(InMeta)), ClassIndex(InClass) {
	}
};

FMallocBinned::FMallocBinned() noexcept
	:PageSize(FPlatformMemory::GetMemoryConstants().PageSize)
{
	static_assert(sizeof(SIZE_T) == 8);
	static_assert(sizeof(Sizes) / sizeof(Sizes[0]) == ClassCount);
	if (PageSize < 16 || (PageSize & (PageSize - 1)) != 0)InvalidUse();
}

FMallocBinned::~FMallocBinned() noexcept
{
	if (AllSpans != nullptr || Stats.ActiveAllocations != 0)
		FPlatformMemory::EmergencyTerminate({ EPlatformMemoryError::InvalidState,0 });
}

void FMallocBinned::AddAvailable(FSpan* Span) noexcept
{
	auto*& Head = Available[Span->ClassIndex];
	Span->AvailablePrev = nullptr;
	Span->AvailableNext = Head;
	if (Head) Head->AvailablePrev = Span;
	Head = Span;
}

void FMallocBinned::RemoveAvailable(FSpan* Span) noexcept
{
	if (Span->AvailablePrev) Span->AvailablePrev->AvailableNext = Span->AvailableNext;
	else Available[Span->ClassIndex] = Span->AvailableNext;
	if (Span->AvailableNext) Span->AvailableNext->AvailablePrev = Span->AvailablePrev;
	Span->AvailablePrev = Span->AvailableNext = nullptr;
}

void FMallocBinned::RegisterPages(FSpan* Span) noexcept
{
	const auto FirstKey = reinterpret_cast<std::uintptr_t>(Span->UserBase) / PageSize;
	for (SIZE_T i = 0;i < Span->EntryCount;++i)
	{
		auto& Entry = Span->Entries[i];
		Entry.Key = FirstKey + i;
		Entry.Span = Span;
		auto*& Head = Directory[Entry.Key % BucketCount];
		Entry.Next = Head;
		Head = &Entry;
	}
}

void FMallocBinned::UnregisterPages(FSpan* Span) noexcept
{
	for (SIZE_T i = 0;i < Span->EntryCount;++i)
	{
		auto* Entry = &Span->Entries[i];
		auto** Link = &Directory[Entry->Key % BucketCount];
		while (*Link && *Link != Entry) Link = &(*Link)->Next;
		if (!*Link) InvalidUse();
		*Link = Entry->Next;
	}
}

FMallocBinned::FSpan* FMallocBinned::CreateSpan(SIZE_T ClassIndex, SIZE_T Size, SIZE_T Alignment) noexcept
{
	const bool Small = ClassIndex != LargeClass;
	SIZE_T DataRequest = SlabTarget;
	if (!Small && !Add(Size, Alignment - 1, DataRequest))return nullptr;
	FPageRegion Data;
	++Stats.PageAllocationCalls;
	if (!FPlatformMemory::TryAllocatePages(DataRequest, Data).Succeeded()) return nullptr;

	const auto Count = Small ? Data.GetSize() / Sizes[ClassIndex] : 0;
	if (Small && (Count == 0 || Count >= NoBlock)) return nullptr;
	const auto EntryCount = Small ? Data.GetSize() / PageSize : 1;
	SIZE_T Total = sizeof(FSpan), RecordOffset = 0, EntryOffset = 0;
	if (!AppendArray(Count, sizeof(FBlockRecord), alignof(FBlockRecord), Total, RecordOffset) ||
		!AppendArray(EntryCount, sizeof(FPageEntry), alignof(FPageEntry), Total, EntryOffset))
		return nullptr;
	FPageRegion Metadata;
	++Stats.PageAllocationCalls;
	if (!FPlatformMemory::TryAllocatePages(Total, Metadata).Succeeded()) return nullptr;

	SIZE_T NewDataBytes = 0, NewMetadataBytes = 0;
	if (!Add(Stats.DataRegionBytes, Data.GetSize(), NewDataBytes) ||
		!Add(Stats.MetadataRegionBytes, Metadata.GetSize(), NewMetadataBytes))return nullptr;
	auto* Storage = static_cast<unsigned char*>(Metadata.GetBase());
	auto* Span = ::new(static_cast<void*>(Storage)) FSpan(std::move(Data), std::move(Metadata), ClassIndex);
	Span->UserBase = static_cast<unsigned char*>(Span->Data.GetBase());
	if (Small)
	{
		Span->Capacity = Sizes[ClassIndex];
		Span->Count = Count;
		Span->FreeHead = 0;
		Span->Records = reinterpret_cast<FBlockRecord*>(Storage + RecordOffset);

		for (SIZE_T i = 0;i < Count;++i)
			::new (static_cast<void*>(&Span->Records[i])) FBlockRecord{
			0,i + 1 == Count ? NoBlock : static_cast<uint32>((i + 1)) };
	}
	else
	{
		const auto Address = reinterpret_cast<uintptr_t>(Span->UserBase);
		const auto Padding = (Alignment - (Address & (Alignment - 1))) & (Alignment - 1);
		Span->UserBase += Padding;
		Span->Capacity = Span->Data.GetSize() - Padding;
	}
	Span->Entries = reinterpret_cast<FPageEntry*>(Storage + EntryOffset);
	Span->EntryCount = EntryCount;
	for (SIZE_T i = 0;i < EntryCount;++i)
		::new (static_cast<void*>(&Span->Entries[i])) FPageEntry{};
	Span->AllNext = AllSpans;
	if (AllSpans) AllSpans->AllPrev = Span;
	AllSpans = Span;
	RegisterPages(Span);
	if (Small) { AddAvailable(Span); ++Stats.SlabCount; }
	else ++Stats.LargeRegionCount;
	Stats.DataRegionBytes = NewDataBytes;
	Stats.MetadataRegionBytes = NewMetadataBytes;
	return Span;
}

void FMallocBinned::DestroySpan(FSpan* Span) noexcept
{
	if (Span->Used != 0)InvalidUse();
	if (Span->ClassIndex != LargeClass) { RemoveAvailable(Span); --Stats.SlabCount; }
	else --Stats.LargeRegionCount;
	UnregisterPages(Span);
	if (Span->AllPrev) Span->AllPrev->AllNext = Span->AllNext;
	else AllSpans = Span->AllNext;
	if (Span->AllNext) Span->AllNext->AllPrev = Span->AllPrev;
	Stats.DataRegionBytes -= Span->Data.GetSize();
	Stats.MetadataRegionBytes -= Span->Metadata.GetSize();
	FPageRegion MetadataOwner(std::move(Span->Metadata));
	Span->~FSpan();
}

FMallocBinned::FSpan* FMallocBinned::FindLiveBlock(const void* Pointer, SIZE_T& Index) const noexcept
{
	const auto Address = reinterpret_cast<std::uintptr_t>(Pointer);
	const auto Key = Address / PageSize;
	for (auto* Entry = Directory[Key % BucketCount]; Entry; Entry = Entry->Next)
	{
		if (Entry->Key != Key) continue;
		FSpan* Span = Entry->Span;
		const auto Base = reinterpret_cast<std::uintptr_t>(Span->UserBase);
		if (Span->ClassIndex == LargeClass)
		{
			if (Address != Base || Span->Used != 1) return nullptr;
			Index = 0;
			return Span;
		}
		if (Address < Base) return nullptr;
		const auto Offset = Address - Base;
		if (Offset % Span->Capacity != 0) return nullptr;
		Index = Offset / Span->Capacity;
		if (Index >= Span->Count || Span->Records[Index].Requested == 0)return nullptr;
		return Span;
	}
	return nullptr;
}

void* FMallocBinned::TryMalloc(SIZE_T Size, uint32 Alignment) noexcept
{
	const auto Align = Normalize(Alignment);
	if (Size == 0)return nullptr;
	SIZE_T NewRequested = 0;
	if (!Add(Stats.RequestedBytes, Size, NewRequested) ||
		Stats.ActiveAllocations == (std::numeric_limits<SIZE_T>::max)()) return nullptr;
	SIZE_T ClassIndex = LargeClass;
	if (Size <= SmallLimit && Align <= 16)
	{
		ClassIndex = 0;
		while (Sizes[ClassIndex] < Size)++ClassIndex;
	}
	FSpan* Span = ClassIndex == LargeClass ? nullptr : Available[ClassIndex];
	if (!Span) Span = CreateSpan(ClassIndex, Size, Align);
	if (!Span) return nullptr;
	SIZE_T NewCapacity = 0;
	if (!Add(Stats.BlockCapacityBytes, Span->Capacity, NewCapacity))
	{
		if (Span->Used == 0)DestroySpan(Span);
		return nullptr;
	}
	unsigned char* Result = Span->UserBase;
	if (ClassIndex != LargeClass)
	{
		const auto Index = Span->FreeHead;
		if (Index == NoBlock || Index >= Span->Count)InvalidUse();
		auto& Record = Span->Records[Index];
		Span->FreeHead = Record.Next;
		Record.Requested = static_cast<uint32>(Size);
		Record.Next = NoBlock;
		Result += static_cast<SIZE_T>(Index) * Span->Capacity;
		++Span->Used;
		if (Span->FreeHead == NoBlock) RemoveAvailable(Span);
	}
	else { Span->Requested = Size;Span->Used = 1; }
	++Stats.ActiveAllocations;
	Stats.RequestedBytes = NewRequested;
	Stats.BlockCapacityBytes = NewCapacity;
	return Result;
}

void FMallocBinned::Free(void* Original) noexcept
{
	if (!Original) return;
	SIZE_T Index = 0;
	auto* Span = FindLiveBlock(Original, Index);
	if (!Span) InvalidUse();
	if (Span->ClassIndex != LargeClass)
	{
		auto& Record = Span->Records[Index];
		Stats.RequestedBytes -= Record.Requested;
		if (Span->Used == Span->Count) AddAvailable(Span);
		Record.Requested = 0;
		Record.Next = Span->FreeHead;
		Span->FreeHead = static_cast<uint32>(Index);
	}
	else { Stats.RequestedBytes -= Span->Requested;Span->Requested = 0; }
	--Span->Used;
	--Stats.ActiveAllocations;
	Stats.BlockCapacityBytes -= Span->Capacity;
	if (Span->Used == 0)DestroySpan(Span);
}

bool FMallocBinned::GetAllocationSize(const void* Original, SIZE_T& Out) const noexcept
{
	Out = 0;
	if (!Original) return false;
	SIZE_T Index = 0;
	auto* Span = FindLiveBlock(Original, Index);
	if (!Span) InvalidUse();
	Out = Span->ClassIndex == LargeClass ? Span->Requested : Span->Records[Index].Requested;
	return true;
}

void* FMallocBinned::TryRealloc(void* Original, SIZE_T Size, uint32 Alignment) noexcept
{
	const auto Align = Normalize(Alignment);
	if (Size == 0) { Free(Original);return nullptr; }
	if (!Original) return TryMalloc(Size, Alignment);
	SIZE_T Index = 0;
	auto* Span = FindLiveBlock(Original, Index);
	if (!Span) InvalidUse();
	const auto OldSize = Span->ClassIndex == LargeClass ? Span->Requested : Span->Records[Index].Requested;
	if (Size <= Span->Capacity && (reinterpret_cast<std::uintptr_t>(Original) & (Align - 1)) == 0)
	{
		SIZE_T NewRequested = 0;
		if (!Add(Stats.RequestedBytes - OldSize, Size, NewRequested))return nullptr;
		Stats.RequestedBytes = NewRequested;
		if (Span->ClassIndex == LargeClass) Span->Requested = Size;
		else Span->Records[Index].Requested = static_cast<uint32>(Size);
		return Original;
	}
	void* Result = TryMalloc(Size, Alignment);
	if (!Result) return nullptr;
	std::memcpy(Result, Original, OldSize < Size ? OldSize : Size);
	Free(Original);
	return Result;
}
