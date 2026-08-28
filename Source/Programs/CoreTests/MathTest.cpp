#include <gtest/gtest.h>
#include "Math/Vector.h"
#include "Math/Matrix.h"

TEST(MathTest, SIMDMemoryAlignmemt)
{
	EXPECT_EQ(alignof(FVector), 16);
	EXPECT_EQ(sizeof(FVector), 16);

	EXPECT_EQ(alignof(FMatrix), 16);
	EXPECT_EQ(sizeof(FMatrix), 64);
}

TEST(MathTest, VectorSIMDOperation)
{
	FVector A(1.0f, 2.0f, 3.0f);
	FVector B(4.0f, 5.0f, 6.0f);

	FVector AddResult = A + B;
	EXPECT_FLOAT_EQ(AddResult.X, 5.0f);
	EXPECT_FLOAT_EQ(AddResult.Y, 7.0f);
	EXPECT_FLOAT_EQ(AddResult.Z, 9.0f);
	
	float DotResult = A.Dot(B);
	EXPECT_FLOAT_EQ(DotResult, 32.0f);
}

TEST(MathTest, MatrixSIMDMultiplication)
{
	FMatrix M1;

	for (int i = 0;i < 4;++i)
	{
		for (int j = 0;j < 4;++j)
		{
			M1.M[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	FMatrix M2;

	float Counter = 1.0f;
	for (int i = 0;i < 4;++i)
	{
		for (int j = 0;j < 4;++j)
		{
			M2.M[i][j] = Counter++;
		}
	}

	FMatrix Result = M2 * M1;

	EXPECT_FLOAT_EQ(Result.M[0][0], 1.0f);
	EXPECT_FLOAT_EQ(Result.M[1][1], 6.0f);
	EXPECT_FLOAT_EQ(Result.M[2][2], 11.0f);
	EXPECT_FLOAT_EQ(Result.M[3][3], 16.0f);
}