#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"

enum class ELogVerbosity :uint8
{
	Fatal,
	Error,
	Warning,
	Display,
	Log,
};

struct FLogCategoryBase
{
	const TCHAR* CategoryName;
	FLogCategoryBase(const TCHAR* InName) :CategoryName(InName) {};
};

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity) \
	extern CORE_API struct FLogCategory##CategoryName : public FLogCategoryBase { \
		FLogCategory##CategoryName() : FLogCategoryBase(TEXT(#CategoryName)){} \
	} CategoryName

#define DEFINE_LOG_CATEGORY(CategoryName) \
	FLogCategory##CategoryName CategoryName

struct CORE_API FLog
{
	static void Logf(const FLogCategoryBase& Category, ELogVerbosity Verbosity, const TCHAR* Format, ...);
};

#define DS_LOG(CategoryName, Verbosity, Format, ...) \
	FLog::Logf(CategoryName, ELogVerbosity::Verbosity, Format, ##__VA_ARGS__)