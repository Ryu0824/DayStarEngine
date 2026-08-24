#pragma once
#include "DelegateInstances.h"
#include "Misc/AssertionMacros.h"

template<typename Signature> class TDelegate;

template <typename RetVal, typename... Args>
class TDelegate<RetVal(Args...)>	
{
public:
	TDelegate() :Payload(nullptr) {}
	~TDelegate() { Unbind(); }

	void Unbind()
	{
		if (Payload)
		{
			delete Payload;
			Payload = nullptr;
		}
	}

	bool IsBound()const { return Payload && Payload->IsBound(); }

	template<typename UserClass, ESPMode Mode>
	void BindSP(const TSharedPtr<UserClass, Mode>& InUserObject, RetVal (UserClass::*InMethod)(Args...))
	{
		Unbind();	
		Payload = new FSPMethodDelegatePayload<UserClass, Mode, RetVal, Args...>(InUserObject, InMethod);
	}

	RetVal Execute(Args... args) const
	{
		check(IsBound());
		return Payload->Execute(std::forward<Args>(args)...);
	}

	void ExecuteIfBound(Args... args)const
	{
		if (IsBound())
		{
			Payload->Execute(std::forward<Args>(args)...);
		}
	}

private:
	IDelegateInstance<RetVal, Args...>* Payload;
};