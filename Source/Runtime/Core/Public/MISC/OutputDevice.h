#pragma once
#include "Containers/UnrealString.h"
#include "Logging/LogMacros.h"

class FOutputDevice
{
public:
	virtual ~FOutPutDevice() = default;

	virtual void Serialize(const TCHAR* Message, ELogVerbosity Verbosity, const class FName& Category) = 0;

	virtual void Flush() {};
};