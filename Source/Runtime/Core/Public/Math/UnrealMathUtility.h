#pragma once
#include "CoreTypes.h"
#include <cmath>

#define PLATFORM_ENABLE_SIMD 1

#if PLATFORM_ENABLE_SIMD
	#include <xmmintrin.h>
	#include <smmintrin.h>
#endif

#if PLATFORM_WINDOWS
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE inline __attribute__((always_inline))
#endif

struct FMath
{
	static FORCEINLINE float Sin(float Value) { return std::sin(Value); }
	static FORCEINLINE float Cos(float Value) { return std::cos(Value); }
	static FORCEINLINE float Tan(float Value) { return std::tan(Value); }

	template<class T>
	static FORCEINLINE T Clamp(const T X, const T Min, const T Max)
	{
		return X < Min ? Min : X < Max ? X : Max;
	}
};