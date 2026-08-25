#pragma once
#include "CoreTypes.h"
#include "Misc/AssertionMacros.h"
#include "Containers/ContainerAllocationPolicies.h"
#include <type_traits>
#include <new>

template<typename InElementType, typename Allocator = FDefaultAllocator>
class TArray
{
public:
	using ElementType = InElementType;

	TArray() :ArrayNum(0), ArrayMax(0) {}
	~TArray() { Empty(); }

	TArray(const TArray& Other)
		:ArrayNum(0), ArrayMax(0)
	{
		if (Other.ArrayNum > 0)
		{
			Reserve(Other.ArrayMax);

			for (int32 i = 0;i < Other.ArrayNum;++i)
			{
				Add(Other.GetData()[i]);
			}
		}
	}

	TArray& operator=(const TArray& Other)
	{
		if (this != &Other)
		{
			Empty();

			if (Other.ArrayNum > 0)
			{
				Reserve(Other.ArrayMax);

				for (int32 i = 0;i < Other.ArrayNum;++i)
				{
					Add(Other.GetData()[i]);
				}
			}
		}
		return *this;
	}

	void Add(const ElementType& Item)
	{
		if (ArrayNum >= ArrayMax)
		{
			Reserve(ArrayMax == 0 ? 4 : ArrayMax * 2);
		}

		// new(Address) -> Heap Memory Allocation X
		new(GetData() + ArrayNum) ElementType(Item);
		ArrayNum++;
	}

	void Empty()
	{
		if (GetData())
		{
			// std::is_trivially_destructible_v is basic data structure comp
			if constexpr (!std::is_trivially_destructible_v<ElementType>)
			{
				for (int32 i = 0;i < ArrayNum;++i)
				{
					(GetData() + i)->~ElementType();
				}
			}
			AllocatorInstance.ResizeAllocation(ArrayMax, 0, sizeof(ElementType));
			ArrayNum = 0;
			ArrayMax = 0;
		}
	}

	void Reserve(int32 NewCapacity)
	{
		if (NewCapacity > ArrayMax)
		{
			AllocatorInstance.ResizeAllocation(ArrayMax, NewCapacity, sizeof(ElementType));
			ArrayMax = NewCapacity;
		}
	}

	int32 Num() const { return ArrayNum; }
	ElementType* GetData() const { return static_cast<ElementType*>(AllocatorInstance.Data); }

private:
	typename Allocator::ForAnyElementType AllocatorInstance;
	int32 ArrayNum;
	int32 ArrayMax;
};