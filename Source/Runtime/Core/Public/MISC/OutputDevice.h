#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"
#include "Logging/LogMacros.h"

struct FLogCategoryBase;

class CORE_API FOutputDevice
{
public:
	virtual ~FOutputDevice() = default;

	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category) = 0;
};