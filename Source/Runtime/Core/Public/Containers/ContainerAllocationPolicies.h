#pragma once
#include "HAL/FMemory.h"

class FDefaultAllocator
{
public:
	class ForAnyElementType
	{
	public:
		ForAnyElementType() : Data(nullptr){}

		void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
		{
			if (NumElements == 0)
			{
				FMemory::Free(Data);
				Data = nullptr;
			}
			else
			{
				Data = FMemory::Realloc(Data, NumElements * NumBytesPerElement);
			}
		}

		void* Data;
	};
};