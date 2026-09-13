#pragma once
#include <cstddef>
#include <cstdlib>
#if defined(_WIN32)
#include <malloc.h>
#endif
namespace
{
	inline void* AllocatedHostAlignedMemory(std::size_t Alignment, std::size_t Size) noexcept
	{
#if defined(_WIN32)
		return ::_aligned_malloc(Size, Alignment);
#else
		return std::aligned_alloc(Alignment, Size);
#endif
	}
	inline void FreeHostAlignedMemory(void* Pointer) noexcept
	{
#if defined(_WIN32)
		::_aligned_free(Pointer);
#else
		std::free(Pointer);
#endif
	}
}