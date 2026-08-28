#pragma once
#include "CoreTypes.h"

class IModuleInterface
{
public:
	virtual ~IModuleInterface() = default;

	virtual void StartupModule(){}

	virtual void ShutdownModule(){}
};

using FInitializeModuleFunctionPtr = IModuleInterface * (*)();

#define IMPLEMENT_MODULE(ModuleClass, ModuleName) \
	extern "C" DLLEXPORT IModuleInterface* InitializeModule() \
	{	\
		return new ModuleClass(); \
	}