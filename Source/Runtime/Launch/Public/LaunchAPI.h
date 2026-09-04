#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
	#ifdef LAUNCH_EXPORT
		#define LAUNCH_API __declspec(dllexport)
	#else
		#define LAUNCH_API __declspec(dllimport)
	#endif
#else
	#define LAUNCH_API __attribute__(((visibility("default"))))
#endif