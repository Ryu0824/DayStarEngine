#pragma once
#include "MISC/OutputDevice.h"

class FOutputDeviceRedirector : public FOutputDevice
{
private:
	TArray<FOutputDevice*> OutputDevices;

public:
	void AddOutputDevice(FOutputDevice* InDevice)
	{
		OutputDevices.Add(InDevice);
	}

	virtual void Serialize(const )
};