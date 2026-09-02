#pragma once
#include "ApplicationCoreAPI.h"
#include "GenericWindow.h"
#include "GenericApplicationMessageHandler.h"
#include "Templates/SharedPointer.h"

class APPLICATIONCORE_API FGenericApplication
{
public:
	virtual ~FGenericApplication() = default;

	virtual void PumpMessages(const float TimeDelta) = 0;

	virtual TSharedPtr<FGenericWindow> MakeWindow(const FGenericWindowDefinition& Definition) = 0;

	virtual void SetMessageHandler(const TSharedPtr<FGenericApplicationMessageHandler>& InMessageHandler)
	{
		MessageHandler = InMessageHandler;
	}

	TSharedPtr<FGenericApplicationMessageHandler> GetMessageHandler()const
	{
		return MessageHandler;
	}

protected:
	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;
};