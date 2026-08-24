#include <gtest/gtest.h>
#include "HAL/FMemory.h"
#include "Delegates/DelegateSignatureImpl.h"
#include "Delegates/MulticastDelegate.h"
#include "Templates/MakeShared.h"

class CoreDelegateTestFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!GMalloc)
		{
			FMemory::SetupMemoryPools();
		}
	}
};

struct FTestListner
{
	int32 HitCount = 0;
	void OnTakeDamage(int32 DamageAmount)
	{
		HitCount += DamageAmount;
	}
};

TEST_F(CoreDelegateTestFixture, SinglecastDelegateLifetimeTest)
{
	TDelegate<void(int32)> OnDamaged;
	{
		TSharedRef<FTestListner> Listener = MakeShared<FTestListner>();
		TSharedPtr<FTestListner> Listener2 = Listener;
		OnDamaged.BindSP(Listener2, &FTestListner::OnTakeDamage);

		EXPECT_TRUE(OnDamaged.IsBound());

		OnDamaged.ExecuteIfBound(10);
		EXPECT_EQ(Listener->HitCount, 10);
	}

	EXPECT_FALSE(OnDamaged.IsBound());

	OnDamaged.ExecuteIfBound(20);
}

TEST_F(CoreDelegateTestFixture, MulticastDelegateDeadListenerTest)
{
	TMulticastDelegate<void(int32)> OnGloabalEvent;

	TSharedRef<FTestListner> AliveListener = MakeShared<FTestListner>();
	TSharedPtr<FTestListner> AliveListener2 = AliveListener;
	OnGloabalEvent.AddSP(AliveListener2, &FTestListner::OnTakeDamage);

	{
		TSharedRef<FTestListner> ShortLivedListener = MakeShared<FTestListner>();
		TSharedPtr<FTestListner> ShortLivedListener2 = ShortLivedListener;
		OnGloabalEvent.AddSP(ShortLivedListener2, &FTestListner::OnTakeDamage);

		OnGloabalEvent.Broadcast(5);
		EXPECT_EQ(AliveListener->HitCount, 5);
		EXPECT_EQ(ShortLivedListener2->HitCount, 5);
	}
	OnGloabalEvent.Broadcast(10);
	EXPECT_EQ(AliveListener->HitCount, 15);
}
