#pragma once
#include <cassert>
#include <iostream>

#if PLATFORM_WINDOWS
	#define PLATFORM_BREAK() __debugbreak()
#else
	#define PLATFORM_BREAK() __builtin_trap()
#endif

#if !UE_BUILD_SHIPPING
	#define check(expr) \
		if(!(expr)) { \
			std::cerr << "[DayStar] Fatal Error : Assertion failed: " << #expr \
					  << " File: "<<__FILE__<<" Line : "<<__LINE__<<std::endl;\
			PLATFORM_BREAK(); \
		}
#else
	#define check(expr)
#endif

#if !UE_BUILD_SHIPPING
	#define ensure(expr) (!!(expr) || \
		([&]() -> bool {\
			std::cerr << "[DayStar] Ensure failed: "<< #expr \
					  << " File: "<<__FILE__<<" Line : "<<__LINE__<<std::endl;\
			return false; \
			}()))
#else
	#define ensure(expr) (!!(expr))
#endif