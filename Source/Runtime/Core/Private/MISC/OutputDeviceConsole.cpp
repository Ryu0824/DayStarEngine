#include "MISC/OutputDeviceConsole.h"
#include "Logging/LogMacros.h"
#include <cstdio>

void FOutputDeviceConsole::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category)
{
	const TCHAR* VerbosityStr = TEXT("LOG");
	if (Verbosity & ELogVerbosity::Fatal) VerbosityStr = TEXT("FATAL");
	else if (Verbosity & ELogVerbosity::Error) VerbosityStr = TEXT("ERROR");
	else if (Verbosity & ELogVerbosity::Warning) VerbosityStr = TEXT("WARNING");

#if PLATFORM_WINDOWS
	wprintf(TEXT("[%s] %s : %s \n"), Category.CategoryName, VerbosityStr, Message);
#else
	printf(TEXT("[%s] %s : %s \n"), Category.CategoryName, VerbosityStr, Message);
#endif
}