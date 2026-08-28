#pragma once
#include "CoreTypes.h"
#include "CoreAPI.h"
#include "Containers/Array.h"
#include "Containers/Unrealstring.h"
#include "Modules/ModuleInterface.h"

struct FModuleInfo
{
	FString ModuleName;
	void* DllHandle = nullptr;
	IModuleInterface* ModuleInstance = nullptr;
};

class CORE_API FModuleManager
{
public:
	static FModuleManager& Get();

	IModuleInterface* LoadModule(const TCHAR* ModuleName);

	void UnloadModule(const TCHAR* ModuleName);

	IModuleInterface* GetModule(const TCHAR* ModuleName);

private:
	FModuleManager() = default;
	TArray<FModuleInfo> Modules;
};