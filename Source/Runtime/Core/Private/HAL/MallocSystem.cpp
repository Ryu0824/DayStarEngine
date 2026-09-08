#include "MallocSystem.h"
#include "SystemHeap.h"
#include "HAL/PlatformMemory.h"
#include <cstdalign>
#include <cstring>
#include <limits>
#include <new>

struct FHeader
{
	void* Base;
	SIZE_T RequestedSize;
	SIZE_T Alignment;
};

static_assert(sizeof(SIZE_T) == 8, "64-bit implementation");

SIZE_T NormalizeAlignment(uint32 Alignment) noexcept
{
	SIZE_T Value = Alignment == 0 ? 16 : Alignment;
	if ((Value & (Value - 1)) != 0)
		FPlatformMemory::EmergencyTerminate({ EPlatformMemoryError::InvalidArgument,0 });
	return Value < alignof(FHeader) ? alignof(FHeader) : Value;
}

bool ComputeTotal(SIZE_T Size, SIZE_T Alignment, SIZE_T& Total) noexcept
{
	constexpr auto Maximum = (std::numeric_limits<SIZE_T>::max)();
	if (Alignment - 1 > Maximum - sizeof(FHeader))return false;
	const auto Overhead = sizeof(FHeader) + Alignment - 1;
	if (Size > Maximum - Overhead) return false;
	Total = Size + Overhead;
	return true;
}

unsigned char* Payload(void* Base, SIZE_T Alignemnt) noexcept
{
	auto* Start = static_cast<unsigned char*>(Base) + sizeof(FHeader);
	const auto Address = reinterpret_cast<std::uintptr_t>(Start);
	const auto Padding = (Alignemnt - (Address & (Alignemnt - 1))) & (Alignemnt - 1);
	return Start + Padding;
}

const FHeader* HeaderOf(const void* Pointer) noexcept
{
	return reinterpret_cast<const FHeader*>(static_cast<const unsigned char*>(Pointer) - sizeof(FHeader));
}

void WriteHeader(unsigned char* Data, void* Base, SIZE_T Size, SIZE_T Alignment) noexcept
{
	::new(static_cast<void*>(Data - sizeof(FHeader))) FHeader{ Base, Size, Alignment };
}

void* FMallocSystem::TryMalloc(SIZE_T Size, uint32 Alignment) noexcept
{
	const auto Align = NormalizeAlignment(Alignment);
	if (Size == 0) return nullptr;
	SIZE_T Total = 0;
	if (!ComputeTotal(Size, Align, Total))return nullptr;
	void* Base = SystemMalloc(Total);
	if (!Base) return nullptr;
	auto* Data = Payload(Base, Align);
	WriteHeader(Data, Base, Size, Align);
	return Data;
}

void* FMallocSystem::TryRealloc(void* Original, SIZE_T Size, uint32 Alignment) noexcept
{
	const auto Align = NormalizeAlignment(Alignment);
	if (Size == 0) { Free(Original); return nullptr; }
	if (!Original) return TryMalloc(Size, Alignment);
	SIZE_T Total = 0;
	if (!ComputeTotal(Size, Align, Total))return nullptr;

	const FHeader Old = *HeaderOf(Original);
	const auto CopySize = Old.RequestedSize < Size ? Old.RequestedSize : Size;
	if (Align != Old.Alignment)
	{
		void* Data = TryMalloc(Size, Alignment);
		if (!Data) return nullptr;
		std::memcpy(Data, Original, CopySize);
		Free(Original);
		return Data;
	}
	if (Size == Old.RequestedSize) return Original;

	const auto OldOffset = static_cast<SIZE_T>(static_cast<unsigned char*>(Original) - static_cast<unsigned char*>(Old.Base));
	void* NewBase = SystemRealloc(Old.Base, Total);
	if (!NewBase) return nullptr;

	auto* NewData = Payload(NewBase, Align);
	auto* OldDataInNewBlock = static_cast<unsigned char*>(NewBase) + OldOffset;
	if (NewData != OldDataInNewBlock)
		std::memmove(NewData, OldDataInNewBlock, CopySize);
	WriteHeader(NewData, NewBase, Size, Align);
	return NewData;
}

void FMallocSystem::Free(void* Original) noexcept
{
	if (Original) SystemFree(HeaderOf(Original)->Base);
}

bool FMallocSystem::GetAllocationSize(const void* Original, SIZE_T& Out) const noexcept
{
	Out = Original ? HeaderOf(Original)->RequestedSize : 0;
	return Original != nullptr;
}