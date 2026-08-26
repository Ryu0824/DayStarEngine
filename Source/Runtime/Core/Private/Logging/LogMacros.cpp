#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include <cstdio>
#include <cstdarg>

DEFINE_LOG_CATEGORY(LogCore);

void ReportAssertFailed(const TCHAR* Expr, const char* File, int Line, const TCHAR* Format, ...)
{
	DS_LOG(LogCore, Fatal, TEXT("Assertion failed: %s [File: %s] [Line: $d]"), ##__VA_ARGS__);
}

void ReportEnsureFailed(const TCHAR* ExprString, const char* File, int Line)
{
	DS_LOG(LogCore, Error, TEXT("Ensure failed: %s. [File: %s] [Line: $d]"), ExprString, File, Line);
	FPlatformMisc::DebugBreak();
}

void FLog::Logf(const FLogCategoryBase& Category, ELogVerbosity Verbosity, const TCHAR* Format, ...)
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
	if (Verbosity == ELogVerbosity::Fatal) VerbosityStr = TEXT("FATAL");
	else if (Verbosity == ELogVerbosity::Error) VerbosityStr = TEXT("ERROR");
	else if (Verbosity == ELogVerbosity::Warning) VerbosityStr = TEXT("WARNING");

#if PLATFORM_WINDOWS
	wprintf(TEXT("[%s] %s: %s\n"), Category.CategoryName, VerbosityStr, Buffer);
#else
	printf(TEXT("[%ls] %ls: %ls\n"), Category.CategoryName, VerbosityStr, Buffer);
#endif
}