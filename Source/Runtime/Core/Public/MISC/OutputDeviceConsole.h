#pragma once
#include "MISC/OutputDevice.h"

class CORE_API FOutputDeviceConsole : public FOutputDevice
{
public:
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category);
};