#pragma once
#include "Logging/LogVerbosity.h"
#include "Logging/LogMacros.h"
#include "HAL/PlatformMisc.h"

struct FDebug
{
	static void Logf_Internal(const TCHAR* Format, ...);
};

extern CORE_API void ReportAssertFailed(const TCHAR* Expr, const char* File, int Line);

extern CORE_API void ReportAssertFailed(const TCHAR* Expr, const char* File, int Line, const TCHAR* Format, ...);

extern CORE_API void ReportEnsureFailed(const TCHAR* Expr, const char* File, int Line);

#ifndef DO_CHECK
#define DO_CHECK 1
#endif

#if DO_CHECK
#define check(expr) \
		do{ \
			if (!(expr)) {\
				ReportAssertFailed(TEXT(#expr),__FILE__,__LINE__); \
				FPlatformMisc::DebugBreak();\
			}\
		} while(false)

#define checkf(expr, format, ...)\
		do{ \
			if (!(expr)) {\
				ReportAssertFailed(TEXT(#expr),__FILE__,__LINE__, format ,##__VA_ARGS__); \
				FPlatformMisc::DebugBreak(); \
			}\
		} while (false)

#define ensure(expr) \
		(!!(expr) || ([](){ \
			static bool bExcuted = false; \
			if (!bExcuted) { \
			bExcuted = true; \
			ReportEnsureFailed(TEXT(#expr),__FILE__,__LINE__);\
			} \
			return false; \
		}()))
#else
#define check(expr)
#define checkf(expr, format);
#define ensure(expr) (!!(expr))
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log);