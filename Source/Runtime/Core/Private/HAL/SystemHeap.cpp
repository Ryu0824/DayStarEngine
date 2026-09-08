#include "SystemHeap.h"
#include <cstdlib>

void* SystemMalloc(SIZE_T Size) noexcept
{
	return std::malloc(Size);
}

void* SystemRealloc(void* Base, SIZE_T Size) noexcept
{
	return std::realloc(Base, Size);
}

void SystemFree(void* Base) noexcept
{
	std::free(Base);
}