#pragma once
#include "ApplicationCoreAPI.h"

class APPLICATIONCORE_API FGenericApplicationMessageHandler
{
public:
	virtual ~FGenericApplicationMessageHandler() = default;

	virtual bool OnKeyDown(int32 KeyCode, uint32 CharacterCode, bool bIsRepeat) { return false; }
	virtual bool OnKeyUp(int32 KeyCode, uint32 CharacterCode, bool bIsRepeat) { return false; }

	virtual bool OnMouseDown(int32 MouseX, int32 MouseY, int32 ButtonId) { return false; }
	virtual bool OnMouseUp(int32 MouseX, int32 MouseY, int32 ButtonId) { return false; }

	virtual void OnWindowClose(){}
};