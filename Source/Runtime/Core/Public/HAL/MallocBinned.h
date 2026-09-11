#pragma once
#include "HAL/FMalloc.h"

struct FBinnedMemoryStats
{
	SIZE_T ActiveAllocations = 0;
	SIZE_T RequestedBytes = 0;
	SIZE_T BlockCapacityBytes = 0;
	SIZE_T DataRegionBytes = 0;
	SIZE_T MetadataRegionBytes = 0;
	SIZE_T SlabCount = 0;
	SIZE_T LargeRegionCount = 0;
	SIZE_T PageAllocationCalls = 0;
};

class CORE_API FMallocBinned final : public FMalloc
{
public:
	FMallocBinned() noexcept;
	~FMallocBinned() noexcept override;
	FMallocBinned(const FMallocBinned&) = delete;
	FMallocBinned& operator=(const FMallocBinned&) = delete;
	FMallocBinned(FMallocBinned&&) = delete;
	FMallocBinned& operator=(FMallocBinned&&) = delete;

	virtual void* TryMalloc(SIZE_T Size, uint32 Alignment = 0) noexcept override;
	virtual void* TryRealloc(void* Original, SIZE_T Size, uint32 Alignment = 0) noexcept override;
	virtual void Free(void* Original) noexcept override;
	virtual bool GetAllocationSize(const void* Original, SIZE_T& Out) const noexcept override;
	virtual bool IsInternallyThreadSafe() const noexcept override { return false; }
	virtual const char* GetDescriptiveName() const noexcept override { return "DayStar single-thread binned prototype"; }
	[[nodiscard]] FBinnedMemoryStats GetStats() const noexcept { return Stats; }

private:
	struct FSpan;
	struct FPageEntry;
	struct FBlockRecord;
	static constexpr SIZE_T ClassCount = 24;
	static constexpr SIZE_T BucketCount = 256;
	SIZE_T PageSize;
	FSpan* Available[ClassCount]{};
	FPageEntry* Directory[BucketCount]{};
	FSpan* AllSpans = nullptr;
	FBinnedMemoryStats Stats{};

	FSpan* CreateSpan(SIZE_T ClassIndex, SIZE_T Size, SIZE_T Alignment) noexcept;
	void DestroySpan(FSpan* Span) noexcept;
	void AddAvailable(FSpan* Span) noexcept;
	void RemoveAvailable(FSpan* Span) noexcept;
	void RegisterPages(FSpan* Span) noexcept;
	void UnregisterPages(FSpan* Span) noexcept;
	FSpan* FindLiveBlock(const void* Pointer, SIZE_T& Index) const noexcept;
};