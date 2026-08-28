#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "MISC/OutputDeviceRedirector.h"
#include <cstdio>
#include <cstdarg>

void FMsg::Logf(const char* File, int Line, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const TCHAR* Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	TCHAR Buffer[1024];
#if PLATFORM_WINDOWS
	_vsnwprintf_s(Buffer, 1024, _TRUNCATE, Format, Args);
#else
	vswprintf(Buffer, 1024, Format, Args);
#endif

	va_end(Args);

	GLog->Serialize(Buffer, Verbosity, Category);
}