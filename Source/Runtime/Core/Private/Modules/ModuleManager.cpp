#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "MISC/AssertionMacros.h"

FModuleManager& FModuleManager::Get()
{
	static FModuleManager Instance;
	return Instance;
}

IModuleInterface* FModuleManager::LoadModule(const TCHAR* ModuleName)
{
	if (IModuleInterface* ExistingModule = GetModule(ModuleName))
	{
		return ExistingModule;
	}

	DS_LOG(LogCore, ELogVerbosity::Display, TEXT("Loading Module : %s"), ModuleName);

	void* Handle = FPlatformProcess::GetDllHandle(ModuleName);
	if (!Handle)
	{
		return nullptr;
	}

	FInitializeModuleFunctionPtr InitFunc = (FInitializeModuleFunctionPtr)FPlatformProcess::GetDllExport(Handle, "IntializeModule");

	if (ensure(InitFunc != nullptr))
	{
		IModuleInterface* ModuleInstance = InitFunc();
		ModuleInstance->StartupModule();

		FModuleInfo Info;
		Info.ModuleName = FString(ModuleName);
		Info.DllHandle = Handle;
		Info.ModuleInstance = ModuleInstance;
		Modules.Add(Info);

		DS_LOG(LogCore, ELogVerbosity::Display, TEXT("Successfully loaded module : %s"), ModuleName);
		return ModuleInstance;
	}

	FPlatformProcess::FreeDllHandle(Handle);
	return nullptr;
}

void FModuleManager::UnloadModule(const TCHAR* ModuleName)
{
	for (int32 i = 0;i < Modules.Num();++i)
	{
		FModuleInfo& Info = Modules[i];

		DS_LOG(LogCore, ELogVerbosity::Display, TEXT("Unloading Module..."));

		if (Info.ModuleInstance)
		{
			Info.ModuleInstance->ShutdownModule();
			delete Info.ModuleInstance;
		}

		FPlatformProcess::FreeDllHandle(Info.DllHandle);

		break;
	}
}

IModuleInterface* FModuleManager::GetModule(const TCHAR* ModuleName)
{
	for (int32 i = 0;i < Modules.Num();++i)
	{
		FModuleInfo& Info = Modules[i];

#if PLATFORM_WINDOWS
		if ((Info.ModuleName == ModuleName) == 0)
#else	
		if ((Info.ModuleName == ModuleName) == 0)
#endif
		{
			return Info.ModuleInstance;
		}
	}

	return nullptr;
}