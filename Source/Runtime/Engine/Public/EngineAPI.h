#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
#ifdef ENGINE_EXPORT
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API __attribute__(((visibility("default"))))
#endif