#pragma once
#include "RHIAPI.h"
#include "MISC/AssertionMacros.h"
#include <atomic>

class RHI_API FRHIResource
{
public:
	FRHIResource() : RefCount(0) {};
	virtual ~FRHIResource() = default;

	uint32 AddRef()const { return ++RefCount; }
	uint32 Release() const
	{
		uint32 Refs = --RefCount;
		if (Refs == 0) { delete this; }
		return Refs;
	}
	uint32 GetRefCount() const { return RefCount; }


private:
	mutable std::atomic<uint32> RefCount;
};

class RHI_API FRHIVertexBuffer : public FRHIResource
{
public:
	FRHIVertexBuffer(uint32 InSize, uint32 InUsage)
		:Size(InSize),Usage(InUsage){ }

	uint32 GetSize()const { return Size; }
	uint32 GetUsage() const { return Usage; }

private:
	uint32 Size;
	uint32 Usage;
};