#include <gtest/gtest.h>
#include <Misc/AssertionMacros.h>
#include <Logging/LogMacros.h>

class CoreAssertionTestFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{

	}
};

TEST_F(CoreAssertionTestFixture, CheckMacroDeathTest)
{
	int32 PlayerHealth = 0;

	EXPECT_DEATH(check(PlayerHealth > 0), "Assertion failed: PlayerHealth > 0");
}

TEST_F(CoreAssertionTestFixture, CheckFMacroDeathTest)
{
	void* CriticalPtr = nullptr;

	EXPECT_DEATH(
		checkf(CriticalPtr != nullptr, TEXT("Critical System pointer is null!")),
		"Critical System pointer is null!"
	);
}

TEST_F(CoreAssertionTestFixture, EnsureLogicalReturnTest)
{
	int32 Ammo = 30;

	if (ensure(Ammo > 0))
	{
		EXPECT_TRUE(true);
	}
	else
	{
		FAIL() << "ENSURE should return true when condition is met.";
	}

#if !DO_CHECK
	if (ensure(Ammo == 0))
	{
		FAIL() << "ENSURE should return false when condition fails.";
	}
	else
	{
		EXPECT_TRUE(true);
	}
#endif
}