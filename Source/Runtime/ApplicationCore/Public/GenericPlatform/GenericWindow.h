#pragma once
#include "ApplicationCoreAPI.h"
#include <memory>

struct FGenericWindowDefinition
{
	float width = 1280.0f;
	float Height = 720.0f;
	const TCHAR* Title = TEXT("DayStar Engine");
	bool bHasOSWindowBorder = true;
};

class APPLICATIONCORE_API FGenericWindow
{
public:
	virtual ~FGenericWindow() = default;

	virtual void Show() = 0;
	virtual void Hide() = 0;

	virtual void* GetOSWindowHandle() const = 0;
};