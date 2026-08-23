#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"
#include "Containers/Array.h"

class CORE_API FString
{
public:
	FString() = default;
	FString(const TCHAR* InStr)
	{
		if (InStr)
		{
			SIZE_T Length = 0;
			while (InStr[Length] != 0) { Length++; }

			Data.Reserve(static_cast<int32>(Length + 1));
			for (SIZE_T i = 0;i <= Length;++i)
			{
				Data.Add(InStr[i]);
			}
		}
	}

	const TCHAR* operator*() const
	{
		return Data.Num() > 0 ? Data.GetData() : TEXT("");
	}

	int32 Len() const { return Data.Num() > 0 ? Data.Num() - 1 : 0; }

private:
	TArray<TCHAR> Data;
};