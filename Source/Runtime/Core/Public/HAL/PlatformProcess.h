#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"

struct CORE_API FPlatformProcess
{
	static void* GetDllHandle(const TCHAR* Filename);

	static void FreeDllHandle(void* DllHandle);

	static void* GetDllExport(void* DllHandle, const char* ProcName);
};