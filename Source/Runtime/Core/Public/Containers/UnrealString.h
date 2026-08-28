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

	FString& operator=(const TCHAR* InStr)
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

		return *this;
	}

	FString& operator=(const FString& Other)
	{
		Data.Empty();

		if (Other.Data.GetData())
		{
			Data.Reserve(Other.Data.Num());

			for (int32 i = 0;i < Other.Data.Num();++i)
			{
				Data.Add(Other.Data[i]);
			}
		}

		return *this;
	}

	const TCHAR* operator*() const
	{
		return Data.Num() > 0 ? Data.GetData() : TEXT("");
	}

	bool operator==(const TCHAR* Instr) const
	{
		for (int32 i = 0;Instr[i] != '\0'; ++i)
		{
			if (Data[i] != Instr[i]) return false;
		}

		return true;
	}

	int32 Len() const { return Data.Num() > 0 ? Data.Num() - 1 : 0; }

private:
	TArray<TCHAR> Data;
};