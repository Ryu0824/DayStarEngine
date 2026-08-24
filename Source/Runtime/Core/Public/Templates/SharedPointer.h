#pragma once
#include "SharedPointerInternals.h"
#include "Misc/AssertionMacros.h"
#include <utility>

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe> class TSharedPtr;
template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe> class TSharedRef;
template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe> class TWeakPtr;

template<typename ObjectType, ESPMode Mode>
class TSharedRef
{
public:
	TSharedRef() = delete;

	explicit TSharedRef(ObjectType* InObject)
		: Object(InObject)
		, ReferenceController(new SharedPointerInternals::FReferenceController<Mode>())
	{
		check(Object != nullptr);
	}

	TSharedRef(const TSharedRef& Other)
		:Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		ReferenceController->AddSharedReference();
	}

	TSharedRef& operator=(const TSharedRef& Other)
	{
		if (this != &Other)
		{
			if (ReferenceController->ReleaseSharedReference())
			{
				delete Object;
				Object = nullptr;
				if (ReferenceController->ReleaseWeakReference())
				{
					delete ReferenceController;
					ReferenceController = nullptr;
				}
			}
			Object = Other.Object;
			ReferenceController = Other.ReferenceController;
			ReferenceController->AddSharedReference();
		}

		return *this;
	}

	~TSharedRef()
	{
		if (ReferenceController->ReleaseSharedReference())
		{
			delete Object;
			Object = nullptr;
			if (ReferenceController->ReleaseWeakReference())
			{
				delete ReferenceController;
				ReferenceController = nullptr;
			}
		}
	}

	ObjectType& Get() const { return *Object; }
	ObjectType* operator->() const { return Object; }
	ObjectType& operator*() const { return *Object; }

private:
	ObjectType* Object;
	SharedPointerInternals::FReferenceController<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode> friend class TSharedPtr;
	template<typename OtherType, ESPMode OtherMode> friend class TWeakPtr;
};

template<typename ObjectType, ESPMode Mode>
class TSharedPtr
{
private:
	TSharedPtr(const TWeakPtr<ObjectType, Mode>& InWeakRef)
		:Object(InWeakRef.Object)
		, ReferenceController(InWeakRef.ReferenceController)
	{
		if (ReferenceController)
		{
			ReferenceController->AddSharedReference();
		}
	}

public:
	TSharedPtr() :Object(nullptr), ReferenceController(nullptr) {}

	TSharedPtr(const TSharedRef<ObjectType, Mode>& InSharedRef)
		:Object(InSharedRef.Object)
		, ReferenceController(InSharedRef.ReferenceController)
	{
		if (ReferenceController)
		{
			ReferenceController->AddSharedReference();
		}
	}

	TSharedPtr(const TSharedPtr& Other)
		:Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		if (ReferenceController)
		{
			ReferenceController->AddSharedReference();
		}
	}

	TSharedPtr& operator=(const TSharedPtr& Other)
	{
		if (this != &Other)
		{
			if (ReferenceController && ReferenceController->ReleaseSharedReference())
			{
				delete Object;
				Object = nullptr;
				if (ReferenceController->ReleaseWeakReference())
				{
					delete ReferenceController;
					ReferenceController = nullptr;
				}
			}

			Object = Other.Object;
			ReferenceController = Other.ReferenceController;

			if (ReferenceController)
			{
				ReferenceController->AddSharedReference();
			}
		}
		return *this;
	}

	~TSharedPtr()
	{
		if (ReferenceController)
		{
			if (ReferenceController->ReleaseSharedReference())
			{
				delete Object;
				Object = nullptr;
				if (ReferenceController->ReleaseWeakReference())
				{
					delete ReferenceController;
					ReferenceController = nullptr;
				}
			}
		}
	}

	ObjectType* Get() const { return Object; }
	ObjectType* operator->() const
	{
		check(Object != nullptr);
		return Object;
	}
	ObjectType& operator*() const { return *Object; }
	bool IsValid() const { return Object != nullptr; }

private:
	ObjectType* Object;
	SharedPointerInternals::FReferenceController<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode> friend class TWeakPtr;
};

template<typename ObjectType, ESPMode Mode>
class TWeakPtr
{
public:
	TWeakPtr() :Object(nullptr), ReferenceController(nullptr) {};

	TWeakPtr(const TSharedPtr<ObjectType, Mode>& InSharedPtr)
		:Object(InSharedPtr.Object)
		, ReferenceController(InSharedPtr.ReferenceController)
	{
		if (ReferenceController)
		{
			ReferenceController->AddWeakReference();
		}
	}

	TWeakPtr(const TWeakPtr& Other)
		:Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		if (ReferenceController)
		{
			ReferenceController->AddWeakReference();
		}
	}

	TWeakPtr& operator=(const TWeakPtr& Other)
	{
		if (this != &Other)
		{
			if (ReferenceController && ReferenceController->ReleaseWeakReference())
			{
				delete ReferenceController;
				ReferenceController = nullptr;
				Object = nullptr;
			}

			Object = Other.Object;
			ReferenceController = Other.ReferenceController;

			if (ReferenceController)
			{
				ReferenceController->AddWeakReference();
			}
		}
		return *this;
	}

	~TWeakPtr()
	{
		if (ReferenceController)
		{
			if (ReferenceController->ReleaseWeakReference())
			{
				delete ReferenceController;
			}
		}
	}

	bool IsValid() const
	{
		return Object != nullptr &&
			ReferenceController != nullptr &&
			ReferenceController->SharedReferenceCount > 0;
	}

	TSharedPtr<ObjectType, Mode> Pin() const
	{
		if (IsValid())
		{
			return TSharedPtr<ObjectType, Mode>(*this);
		}
		return TSharedPtr<ObjectType, Mode>();
	}

private:
	ObjectType* Object;
	SharedPointerInternals::FReferenceController<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode> friend class TSharedPtr;
};