#pragma once
#include <cstddef>

extern thread_local bool GMemoryTestFailNextMalloc;
extern thread_local bool GMemoryTestFailNextRealloc;
std::size_t GetMemoryTestLiveCount();
std::size_t GetMemoryShiftedReallocs();