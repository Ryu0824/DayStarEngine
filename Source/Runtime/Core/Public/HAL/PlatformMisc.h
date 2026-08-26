#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"

struct CORE_API FPlatformMisc
{
	static void DebugBreak()
	{
#if PLATFORM_WINDOWS
		__debugbreak();
#else
		__builtin_trap();
#endif
	}
};