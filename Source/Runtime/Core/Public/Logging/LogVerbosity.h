#pragma once
#include "CoreTypes.h"

namespace ELogVerbosity
{
	enum Type : uint8
	{
		Fatal = 1 << 0,
		Error = 1 << 1,
		Warning = 1 << 2,
		Display = 1 << 3,
		Log = 1 << 4,
	};
}