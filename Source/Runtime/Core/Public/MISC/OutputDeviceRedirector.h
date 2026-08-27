#pragma once
#include "MISC/OutputDevice.h"
#include "Containers/Array.h"

class CORE_API FOutputDeviceRedirector : public FOutputDevice
{
public:
	void AddOutputDevice(FOutputDevice* InDevice);
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category);

private:
	TArray<FOutputDevice*> BufferDevices;
};

extern CORE_API FOutputDeviceRedirector* GLog;