#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
	#ifdef CORE_EXPORT
		#define CORE_API __declspec(dllexport)
	#else
		#define CORE_API __declspec(dllimport)
	#endif
#else
	#define CORE_API __attribute__(((visibility("default"))))
#endif