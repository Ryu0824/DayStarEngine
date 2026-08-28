#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "MISC/AssertionMacros.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void* FPlatformProcess::GetDllHandle(const TCHAR* Filename)
{
#if PLATFORM_WINDOWS
	void* Handle = LoadLibraryW(Filename);
#else
	void* Handle = dlopen(Filename, RTLD_LAZY | RTLD_LOCAL);
#endif

	if (!Handle)
	{
		DS_LOG(LogCore, ELogVerbosity::Error, TEXT("Failed to load DLL : %s"), Filename);
	}

	return Handle;
}

void FPlatformProcess::FreeDllHandle(void* DllHandle)
{
	if (DllHandle)
	{
#if PLATFORM_WINDOWS
		FreeLibrary((HMODULE)DllHandle);
#else
		dlclose(DllHandle);
#endif
	}
}

void* FPlatformProcess::GetDllExport(void* DllHandle, const char* ProcName)
{
	if (!DllHandle || !ProcName) return nullptr;

#if PLATFORM_WINDOWS
	return (void*)GetProcAddress((HMODULE)DllHandle, ProcName);
#else
	return dlsym(DllHandle, ProcName);
#endif
}