#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"
#include "Logging/LogVerbosity.h"
#include "HAL/PlatformMisc.h"

struct FLogCategoryBase
{
	const TCHAR* CategoryName;
	FLogCategoryBase(const TCHAR* InName) :CategoryName(InName) {};
};

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity) \
	extern struct FLogCategory##CategoryName : public FLogCategoryBase { \
		FLogCategory##CategoryName() : FLogCategoryBase(TEXT(#CategoryName)){} \
	} CategoryName

#define DEFINE_LOG_CATEGORY(CategoryName) \
	FLogCategory##CategoryName CategoryName

struct CORE_API FMsg
{
	static void Logf(const char* File, int Line, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const TCHAR* Format, ...);
};

#define DS_LOG(CategoryName, Verbosity, Format, ...) \
	do \
	{ \
		FMsg::Logf(__FILE__, __LINE__, CategoryName, Verbosity, Format, ##__VA_ARGS__); \
		\
		if (Verbosity & ELogVerbosity::Fatal)\
		{\
			FPlatformMisc::DebugBreak();\
		}\
	}while (false);