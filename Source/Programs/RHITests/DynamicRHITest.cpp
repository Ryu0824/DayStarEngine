#include <gtest/gtest.h>
#include "DynamicRHI.h"
#include "RHIResources.h"

class FMockVertexBuffer : public FRHIVertexBuffer
{
public:
	bool* bDestroyedFlag;

	FMockVertexBuffer(uint32 Size, uint32 Usage, bool* OutDestroyedFlag)
		:FRHIVertexBuffer(Size,Usage),bDestroyedFlag(OutDestroyedFlag)
	{
		if (bDestroyedFlag) *bDestroyedFlag = false;
	}

	virtual ~FMockVertexBuffer() override
	{
		if (bDestroyedFlag) *bDestroyedFlag = true;
	}
};

class FMockDynamicRHI : public FDynamicRHI
{
public:
	bool bInitCalled = false;
	bool bDrawCalled = false;
	ERHIPrimitiveType LastPrimitiveType;

	virtual void Init() override { bInitCalled = true; }
	virtual void Shutdown() override {}

	virtual FRHIVertexBuffer* RHICreateVertexBuffer(uint32 Size, uint32 Usage, const void* CreateData) override
	{
		return new FMockVertexBuffer(Size, Usage, nullptr);
	}

	virtual void RHISetViewPort(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) {};
	virtual void RHISetVertexBuffer(FRHIVertexBuffer* VertexBuffer, uint32 StreamIndex, uint32 Offset) {};

	virtual void RHIDrawPrimitive(ERHIPrimitiveType PrimitiveType, uint32 BaseVertexIndex, uint32 NumPrimtivies, uint32 NumInstance)
	{
		bDrawCalled = true;
		LastPrimitiveType = PrimitiveType;
	}
};

TEST(RHITest, ResourceReferenceCounting)
{
	bool bIsDestroyed = false;
	FMockVertexBuffer* VBO = new FMockVertexBuffer(1024, 0, &bIsDestroyed);

	EXPECT_EQ(VBO->GetRefCount(), 0);

	VBO->AddRef();
	EXPECT_EQ(VBO->GetRefCount(), 1);

	VBO->AddRef();
	EXPECT_EQ(VBO->GetRefCount(), 2);

	VBO->Release();
	EXPECT_EQ(VBO->GetRefCount(), 1);
	EXPECT_FALSE(bIsDestroyed);

	VBO->Release();
	EXPECT_TRUE(bIsDestroyed);
}

TEST(RHITest, DynamicRHIRouting)
{
	FMockDynamicRHI MockRHI;
	GDynamicRHI = &MockRHI;

	GDynamicRHI->Init();
	EXPECT_TRUE(MockRHI.bInitCalled);

	GDynamicRHI->RHIDrawPrimitive(ERHIPrimitiveType::TriangleList, 0, 1, 1);
	EXPECT_TRUE(MockRHI.bDrawCalled);
	EXPECT_EQ(MockRHI.LastPrimitiveType, ERHIPrimitiveType::TriangleList);

	GDynamicRHI = nullptr;
}