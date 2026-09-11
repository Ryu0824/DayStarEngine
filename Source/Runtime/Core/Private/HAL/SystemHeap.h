#pragma once
#include "CoreTypes.h"

void* SystemMalloc(SIZE_T Size) noexcept;
void* SystemRealloc(void* Base, SIZE_T Size) noexcept;
void SystemFree(void* Base) noexcept;