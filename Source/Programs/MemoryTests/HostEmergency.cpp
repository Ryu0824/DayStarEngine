#include "HAL/PlatformMemory.h"
#include <cstdlib>

[[noreturn]] void FPlatformMemory::EmergencyTerminate(FPlatformMemoryResult Error) noexcept
{
	std::_Exit(90 + static_cast<int>(Error.Error));
}