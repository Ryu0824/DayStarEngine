#pragma once
#include <cstdint>
#include <cstddef>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef size_t SIZE_T;

#if defined(_WIN32)
	#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
	#define PLATFORM_MAC 1
#else
	#define PLATFORM_LINUX 1
#endif

#if PLATFORM_WINDOWS
typedef wchar_t TCHAR;
#else
typedef char16_t TCHAR;
#endif

#define TEXT(x) L##x