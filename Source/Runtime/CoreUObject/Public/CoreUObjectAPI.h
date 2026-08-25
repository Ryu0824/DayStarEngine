#pragma once
#include "CoreTypes.h"

#if PLATFORM_WINDOWS
#ifdef COREUOBJECT_EXPORT
#define COREUOBJECT_API __declspec(dllexport)
#else
#define COREUOBJECT_API __declspec(dllimport)
#endif
#else
#define COREUOBJECT_API __attribute__(((visibility("default"))))
#endif