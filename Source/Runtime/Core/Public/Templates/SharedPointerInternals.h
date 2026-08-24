#pragma once
#include "CoreTypes.h"
#include "HAL/FMemory.h"
#include <atomic>

enum class ESPMode
{
	NotThreadSafe,
	ThreadSafe
};

namespace SharedPointerInternals
{
	struct FReferenceControllerBase
	{
		void* operator new(SIZE_T Size) { return FMemory::Malloc(Size); }
		void operator delete(void* Ptr) { FMemory::Free(Ptr); }
	};

	template<ESPMode Mode>
	struct FReferenceController;

	template<>
	struct FReferenceController<ESPMode::NotThreadSafe> : public FReferenceControllerBase
	{
		int32 SharedReferenceCount = 1;
		int32 WeakReferenceCount = 1;

		void AddSharedReference() { ++SharedReferenceCount; }
		bool ReleaseSharedReference() { return --SharedReferenceCount == 0; }
		void AddWeakReference() { ++WeakReferenceCount; }
		bool ReleaseWeakReference() { return --WeakReferenceCount == 0; }
	};

	template<>
	struct FReferenceController<ESPMode::ThreadSafe> : public FReferenceControllerBase
	{
		std::atomic<int32> SharedReferenceCount{ 1 };
		std::atomic<int32> WeakReferenceCount{ 1 };

		void AddSharedReference() { SharedReferenceCount.fetch_add(1, std::memory_order_relaxed); }
		bool ReleaseSharedReference() { return SharedReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1; }
		void AddWeakReference() { WeakReferenceCount.fetch_add(1, std::memory_order_relaxed); }
		bool ReleaseWeakReference() { return WeakReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1; }
	};
}