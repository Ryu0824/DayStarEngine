#pragma once
#include "HAL/FMemory.h"
#include "Templates/SharedPointer.h"

template<typename RetVal, typename... Args>
class IDelegateInstance
{
public:
	virtual ~IDelegateInstance() = default;

	void* operator new(SIZE_T Size) { return FMemory::Malloc(Size); }
	void operator delete(void* Ptr) { FMemory::Free(Ptr); }

	virtual RetVal Execute(Args... args) const = 0;
	virtual bool IsBound() const = 0;
};

template<typename UserClass, ESPMode Mode, typename RetVal, typename... Args>
class FSPMethodDelegatePayload : public IDelegateInstance<RetVal, Args...>
{
public:
	using MethodPtr = RetVal (UserClass::*)(Args...);

	FSPMethodDelegatePayload(const TSharedPtr<UserClass, Mode>& InUserObject, MethodPtr InMethod)
		:UserObjectContext(InUserObject)
		, Method(InMethod)
	{
	}

	virtual bool IsBound() const override
	{
		return UserObjectContext.IsValid();
	}

	virtual RetVal Execute(Args... args) const override
	{
		TSharedPtr<UserClass, Mode> PinnedObject = UserObjectContext.Pin();
		if (PinnedObject.IsValid())
		{
			return (PinnedObject.Get()->*Method)(std::forward<Args>(args)...);
		}

		return RetVal();
	}


private:
	TWeakPtr<UserClass, Mode> UserObjectContext;
	MethodPtr Method;
};