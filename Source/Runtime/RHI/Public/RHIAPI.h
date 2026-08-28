#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
#ifdef RHI_EXPORT
#define RHI_API __declspec(dllexport)
#else
#define RHI_API __declspec(dllimport)
#endif
#else
#define RHI_API __attribute__(((visibility("default"))))
#endif