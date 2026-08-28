#pragma once
#include "RHIResources.h"

enum class ERHIPrimitiveType : uint8
{
	TriangleList,
	TriangleStrip,
	LineList,
};

class RHI_API FDynamicRHI
{
public:
	virtual ~FDynamicRHI() = default;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	virtual FRHIVertexBuffer* RHICreateVertexBuffer(uint32 Size, uint32 Usage, const void* CreateData) = 0;

	virtual void RHISetViewPort(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) = 0;
	virtual void RHISetVertexBuffer(FRHIVertexBuffer* VertexBuffer, uint32 StreamIndex, uint32 Offset) = 0;

	virtual void RHIDrawPrimitive(ERHIPrimitiveType PrimitiveType, uint32 BaseVertexIndex, uint32 NumPrimtivies, uint32 NumInstance) = 0;
};

extern RHI_API FDynamicRHI* GDynamicRHI;