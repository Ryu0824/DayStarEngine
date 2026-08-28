#pragma once
#include "Math/UnrealMathUtility.h"

struct alignas(16) FVector
{
	union
	{
		struct { float X, Y, Z, W; };
		float V[4];

#if PLATFORM_ENABLE_SIMD
		__m128 Vector;
#endif
	};

	FORCEINLINE FVector() :X(0.f), Y(0.f), Z(0.f), W(0.f) {};
	FORCEINLINE FVector(float InX, float InY, float InZ) :X(InX), Y(InY), Z(InZ), W(0.f) {};
#if PLATFORM_ENABLE_SIMD
	FORCEINLINE FVector(__m128 InVector) :Vector(InVector) {}
#endif

	FORCEINLINE FVector operator+(const FVector& Other) const
	{
#if PLATFORM_ENABLE_SIMD
		return FVector(_mm_add_ps(Vector, Other.Vector));
#else
		return FVector(X + Other.X, Y + Other.Y, Z + Other.Z);
#endif
	}

	FORCEINLINE float Dot(const FVector& Other) const
	{
#if PLATFORM_ENABLE_SIMD
		__m128 DotVec = _mm_dp_ps(Vector, Other.Vector, 0x7F);
		return _mm_cvtss_f32(DotVec);
#else
		return (X * Other.X) + (Y * Other.Y) + (Z * Other.Z);
#endif
	}
};