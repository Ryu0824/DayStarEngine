#include "MISC/AssertionMacros.h"
#include <cstdio>
#include <cstdarg>

DEFINE_LOG_CATEGORY(LogCore);

void ReportAssertFailed(const TCHAR* Expr, const char* File, int Line)
{
	FDebug::Logf_Internal(TEXT("Assertion failed: %s [File: %hs] [Line: %d]"), Expr, File, Line);
}

void ReportAssertFailed(const TCHAR* Expr, const char* File, int Line, const TCHAR* Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	TCHAR UserMsg[1024];
#if PLATFORM_WINDOWS
	_vsnwprintf_s(UserMsg, 1024, _TRUNCATE, Format, Args);
#else
	vswprintf(UserMsg, 1024, Format, Args);
#endif

	va_end(Args);

	FDebug::Logf_Internal(TEXT("Assertion failed: %s [File: %hs] [Line: %d] \n Reason: %s"), Expr, File, Line, UserMsg);
}

void ReportEnsureFailed(const TCHAR* Expr, const char* File, int Line)
{
	FDebug::Logf_Internal(TEXT("Ensure failed: %s. [File: %s] [Line: %d]"));
}

void FDebug::Logf_Internal(const TCHAR* Format, ...)
{
	va_list Args;
	va_start(Args, Format);
	TCHAR Buffer[2048];
#if PLATFORM_WINDOWS
	_vsnwprintf_s(Buffer, 2048, _TRUNCATE, Format, Args);
#else
	vswprintf(Buffer, 2048, Format, Args);
#endif
	va_end(Args);

#if PLATFORM_WINDOWS
	wprintf(TEXT("[CRITICAL] %s \n"), Buffer);
#else
	printf(TEXT("[CRITICAL] %ls \n"), Buffer);
#endif
}