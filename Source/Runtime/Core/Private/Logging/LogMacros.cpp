#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
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

	const TCHAR* VerbosityStr = TEXT("LOG");
	if (Verbosity & ELogVerbosity::Fatal) VerbosityStr = TEXT("FATAL");
	else if (Verbosity & ELogVerbosity::Error) VerbosityStr = TEXT("ERROR");
	else if (Verbosity & ELogVerbosity::Warning) VerbosityStr = TEXT("WARNING");

#if PLATFORM_WINDOWS
	wprintf(TEXT("[%s] %s: %s\n"), Category.CategoryName, VerbosityStr, Buffer);
#else
	printf(TEXT("[%ls] %ls: %ls\n"), Category.CategoryName, VerbosityStr, Buffer);
#endif
}