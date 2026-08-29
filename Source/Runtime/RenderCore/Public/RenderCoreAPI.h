#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
#ifdef RENDERCORE_EXPORT
#define RENDERCORE_API __declspec(dllexport)
#else
#define RENDERCORE_API __declspec(dllimport)
#endif
#else
#define RENDERCORE_API __attribute__(((visibility("default"))))
#endif