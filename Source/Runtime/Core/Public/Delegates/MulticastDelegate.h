#pragma once
#include "DelegateInstances.h"
#include "Containers/Array.h"

template<typename Signature> class TMulticastDelegate;

template<typename... Args>
class TMulticastDelegate<void(Args...)>
{
public:
	~TMulticastDelegate() { Clear(); }

	void Clear()
	{
		for (int32 i = 0;i < Payloads.Num();++i)
		{
			delete Payloads.GetData()[i];
		}
		Payloads.Empty();
	}

	template<typename UserClass, ESPMode Mode>
	void AddSP(const TSharedPtr<UserClass, Mode>& InUserObject, void (UserClass::* InMethod)(Args...))
	{
		auto* NewPayload = new FSPMethodDelegatePayload<UserClass, Mode, void, Args...>(InUserObject, InMethod);
		Payloads.Add(NewPayload);
	}

	void Broadcast(Args... args)
	{
		for (int32 i = 0;i < Payloads.Num();++i )
		{
			if (Payloads.GetData()[i]->IsBound())
			{
				Payloads.GetData()[i]->Execute(std::forward<Args>(args)...);
			}
			else
			{
				delete Payloads.GetData()[i];
				Payloads.GetData()[i] = nullptr;
			}
		}
	}

private:
	TArray<IDelegateInstance<void, Args...>*> Payloads;
};