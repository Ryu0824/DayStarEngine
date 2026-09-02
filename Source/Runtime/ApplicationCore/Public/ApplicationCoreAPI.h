#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
	#ifdef APPLICATIONCORE_EXPORT
		#define APPLICATIONCORE_API __declspec(dllexport)
	#else
		#define APPLICATIONCORE_API __declspec(dllimport)
	#endif
#else
	#define APPLICATIONCORE_API __attribute__(((visibility("default"))))
#endif