#pragma once
#include "CoreTypes.h"
#include "GenericPlatform/GenericApplication.h"
#include "Templates/SharedPointer.h"

class FEngineLoop
{
public:
	FEngineLoop() = default;
	~FEngineLoop() = default;

	int32 PreInit(const TCHAR* CmdLine);
	int32 Init();
	void Tick();
	void Exit();

private:
	TSharedPtr<FGenericApplication, ESPMode::ThreadSafe> Application;
	TSharedPtr<FGenericWindow, ESPMode::ThreadSafe> MainWindow;
};