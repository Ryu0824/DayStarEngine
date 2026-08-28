#pragma once
#include "Math/Vector.h"

struct alignas(16) FMatrix
{
	union
	{
		float M[4][4];
#if PLATFORM_ENABLE_SIMD
		__m128 Rows[4];
#endif
	};

	FORCEINLINE FMatrix operator*(const FMatrix& Other)const
	{
		FMatrix Result;
#if PLATFORM_ENABLE_SIMD
		for (int i = 0;i < 4;++i)
		{
			__m128 xxxx = _mm_shuffle_ps(Rows[i], Rows[i], _MM_SHUFFLE(0, 0, 0, 0));
			__m128 vResult = _mm_mul_ps(xxxx, Other.Rows[0]);

			__m128 yyyy = _mm_shuffle_ps(Rows[i], Rows[i], _MM_SHUFFLE(1, 1, 1, 1));
			vResult = _mm_add_ps(vResult, _mm_mul_ps(yyyy, Other.Rows[1]));

			__m128 zzzz = _mm_shuffle_ps(Rows[i], Rows[i], _MM_SHUFFLE(2, 2, 2, 2));
			vResult = _mm_add_ps(vResult, _mm_mul_ps(zzzz, Other.Rows[2]));

			__m128 wwww = _mm_shuffle_ps(Rows[i], Rows[i], _MM_SHUFFLE(3, 3, 3, 3));
			Result.Rows[i] = _mm_add_ps(vResult, _mm_mul_ps(wwww, Other.Rows[3]));
		}
#else
		for (int x = 0;x < 4;++x)
		{
			for (int y = 0;y < 4;++y)
			{
				Result.M[x][y] = 0.0f;
				for (int i = 0;i < 4;++i)
				{
					Result.M[x][y] += M[x][i] * Other.M[i][y];
				}
			}
		}
#endif
		return Result;
	}
};