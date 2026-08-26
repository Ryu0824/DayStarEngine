#pragma once
#include "Logging/LogMacros.h"
#include "HAL/PlatformMisc.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log);

extern CORE_API void ReportAssertFailed(const char* ExprString, const char* File, int Line);

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
				DS_LOG(LogCore, Fatal, TEXT("Assertion failed: %s. "), TEXT(#expr), ##__VA_ARGS__); \
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