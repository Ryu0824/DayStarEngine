#pragma once
#include "CoreTypes.h"

class FMalloc
{
public:
	virtual ~FMalloc() = default;

	virtual void* Malloc(SIZE_T Count, uint32 Alignment = 8) = 0;
	virtual void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 8) = 0;
	virtual void Free(void* Original) = 0;

	virtual const char* GetDescriptiveName()const = 0;
};