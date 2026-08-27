#include "MISC/OutputDeviceRedirector.h"


FOutputDeviceRedirector* GLog = nullptr;

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* Device)
{
	if (Device)
	{
		BufferDevices.Add(Device);
	}
}

void FOutputDeviceRedirector::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FLogCategoryBase& Category)
{
	for (int32 i = 0;i < BufferDevices.Num();++i)
	{
		if (FOutputDevice* Device = BufferDevices[i])
		{
			Device->Serialize(Message, Verbosity, Category);
		}
	}
}