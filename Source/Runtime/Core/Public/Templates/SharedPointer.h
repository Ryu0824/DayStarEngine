#pragma once
#include "SharedPointerInternals.h"
#include "Misc/AssertionMacros.h"
#include <utility>
#include <type_traits>

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe>
class TSharedPtr;

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe>
class TSharedRef;

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe>
class TWeakPtr;

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe, typename... ArgTypes>
TSharedRef<ObjectType, Mode> MakeShared(ArgTypes&&... Args);

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedPtr<CastToType, Mode> StaticCastSharedPtr(const TSharedPtr<CastFromType, Mode>& InSharedPtr);

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedRef<CastToType, Mode> StaticCastSharedRef(const TSharedRef<CastFromType, Mode>& InSharedRef);

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedPtr<CastToType, Mode> ConstCastSharedPtr(const TSharedPtr<CastFromType, Mode>& InSharedPtr);

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedRef<CastToType, Mode> ConstCastSharedRef(const TSharedRef<CastFromType, Mode>& InSharedRef);

namespace SharedPointerInternals
{
	template<typename PointerType>
	void CheckSharedRefObject(PointerType* InObject)
	{
		check(InObject != nullptr);

		if (InObject == nullptr)
		{
			std::terminate();
		}
	}
}

template<typename ObjectType, ESPMode Mode>
class TSharedRef
{
public:
	// TSharedRef is not avilable that raw pointer is nullptr
	TSharedRef() = delete;

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	explicit TSharedRef(OtherType* InObject)
		: Object(InObject)
		, ReferenceController(nullptr)
	{
		SharedPointerInternals::CheckSharedRefObject(InObject);
		ReferenceController =
			SharedPointerInternals::CreateReferenceController<OtherType, Mode>(
				InObject,
				std::default_delete<OtherType>{});
	}

	template<typename OtherType, typename DeleterType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedRef(OtherType* InObject, DeleterType&& InDeleter)
		: Object(InObject)
		, ReferenceController(nullptr)
	{
		SharedPointerInternals::CheckSharedRefObject(InObject);
		ReferenceController =
			SharedPointerInternals::CreateReferenceController<OtherType, Mode>(
				InObject,
				std::forward<DeleterType>(InDeleter));
	}

	TSharedRef(const TSharedRef& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		ReferenceController->AddSharedReference();
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedRef(const TSharedRef<OtherType, Mode>& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		ReferenceController->AddSharedReference();
	}

	TSharedRef(TSharedRef&& Other) noexcept
		: Object(std::exchange(Other.Object, nullptr))
		, ReferenceController(std::exchange(Other.ReferenceController, nullptr))
	{
	}

	template<typename OtherType>
		requires std::is_convertible<OtherType*, ObjectType*>
	TSharedRef(TSharedRef<OtherType, Mode>&& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		Other.Object = nullptr;
		Other.ReferenceController = nullptr;
	}

	TSharedRef& operator=(const TSharedRef& Other) noexcept
	{
		if (this != &Other)
		{
			TSharedRef Temporary(Other);
			Swap(Temporary);
		}

		return *this;
	}

	~TSharedRef()
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->ReleaseSharedReference();
		}
	}

	[[nodiscard]] ObjectType& Get() const
	{
		SharedPointerInternals::CheckSharedRefObject(Object);
		return *Object;
	}

	[[nodiscard]] ObjectType* operator->() const
	{
		SharedPointerInternals::CheckSharedRefObject(Object);
		return Object;
	}

	[[nodiscard]] ObjectType& operator*() const
	{
		return Get();
	}

	[[nodiscard]] int32 GetSharedReferenceCount() const noexcept
	{
		return ReferenceController != nullptr
			? ReferenceController->GetSharedReferenceCount()
			: 0;
	}

	[[nodiscard]] bool IsUnique() const noexcept
	{
		return ReferenceController != nullptr && ReferenceController->IsUnique();
	}

	[[nodiscard]] TSharedPtr<ObjectType, Mode> ToSharedPtr() const noexcept;
	[[nodiscard]] TWeakPtr<ObjectType, Mode> ToWeakPtr() const noexcept;

	void Swap(TSharedRef& Other) noexcept
	{
		std::swap(Object, Other.Object);
		std::swap(ReferenceController, Other.ReferenceController);
	}

private:
	TSharedRef(
		ObjectType* Inobject,
		SharedPointerInternals::FReferenceControllerBase<Mode>* InReferenceController,
		SharedPointerInternals::FAdoptControllerTag) noexcept
		: Object(Inobject)
		, ReferenceController(InReferenceController)
	{
		SharedPointerInternals::CheckSharedRefObject(Object);
	}

	TSharedRef(
		ObjectType* Inobject,
		SharedPointerInternals::FReferenceControllerBase<Mode>* InReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag) noexcept
		: Object(Inobject)
		, ReferenceController(InReferenceController)
	{
		SharedPointerInternals::CheckSharedRefObject(Object);
		ReferenceController->AddSharedReference();
	}

	ObjectType* Object;
	SharedPointerInternals::FReferenceControllerBase<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedRef;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedPtr;

	template<typename OtherType, ESPMode OtherMode>
	friend class TWeakPtr;

	template<typename OtherType, ESPMode OtherMode, typename... ArgTypes>
	friend TSharedRef<OtherType, OtherMode> MakeShared(ArgTypes&&... Args);

	template<typename CastToType, typename CastFromType, ESPMode OtherMode>
	friend TSharedRef<CastToType, OtherMode> StaticCastSharedRef(
		const TSharedRef<CastFromType, OtherMode>& InSharedRef);

	template<typename CastToType, typename CastFromType, ESPMode OtherMode>
	friend TSharedRef<CastToType, OtherMode> ConstCastSharedRef(
		const TSharedRef<CastFromType, OtherMode>& InSharedRef);
};

template<typename ObjectType, ESPMode Mode>
class TSharedPtr
{
public:
	TSharedPtr() noexcept
		: Object(nullptr)
		, ReferenceController(nullptr)
	{
	}

	TSharedPtr(std::nullptr_t) noexcept
		: TSharedPtr()
	{
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	explicit TSharedPtr(OtherType* InObject)
		: Object(InObject)
		, ReferenceController(nullptr)
	{
		if (InObject != nullptr)
		{
			ReferenceController =
				SharedPointerInternals::CreateReferenceController<OtherType, Mode>(
					InObject,
					std::default_delete<OtherType>{});
		}
	}

	template<typename OtherType, typename DeleterType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr(OtherType* InObject, DeleterType&& InDeleter)
		: Object(InObject)
		, ReferenceController(nullptr)
	{
		if (InObject != nullptr)
		{
			ReferenceController =
				SharedPointerInternals::CreateReferenceController<OtherType, Mode>(
					InObject,
					std::forward<DeleterType>(InDeleter));
		}
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr(const TSharedRef<OtherType, Mode>& InSharedRef) noexcept
		: Object(InSharedRef.Object)
		, ReferenceController(InSharedRef.ReferenceController)
	{
		ReferenceController->AddSharedReference();
	}

	TSharedPtr(const TSharedPtr& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->AddSharedReference();
		}
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr(const TSharedPtr<OtherType, Mode>& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->AddSharedReference();
		}
	}

	TSharedPtr(TSharedPtr&& Other) noexcept
		: Object(std::exchange(Other.Object, nullptr))
		, ReferenceController(std::exchange(Other.ReferenceController, nullptr))
	{
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr(TSharedPtr<OtherType, Mode>&& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		Other.Object = nullptr;
		Other.ReferenceController = nullptr;
	}

	TSharedPtr& operator=(const TSharedPtr& Other) noexcept
	{
		if (this != &Other)
		{
			TSharedPtr Temporary(Other);
			Swap(Temporary);
		}
		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr& operator=(const TSharedPtr<OtherType, Mode>& Other) noexcept
	{
		TSharedPtr Temporary(Other);
		Swap(Temporary);
		return *this;
	}

	TSharedPtr& operator=(TSharedPtr&& Other) noexcept
	{
		if (this != &Other)
		{
			TSharedPtr Tempoary(std::move(Other));
			Swap(Tempoary);
		}

		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr& operator=(const TSharedPtr<OtherType, Mode>&& Other) noexcept
	{
		TSharedPtr Temporary(std::move(Other));
		Swap(Temporary);
		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TSharedPtr& operator=(const TSharedRef<OtherType, Mode>& Other) noexcept
	{
		TSharedPtr Temporary(Other);
		Swap(Temporary);
		return *this;
	}

	TSharedPtr& operator=(std::nullptr_t) noexcept
	{
		Reset();
		return *this;
	}

	~TSharedPtr()
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->ReleaseSharedReference();
		}
	}

	void Reset() noexcept
	{
		TSharedPtr Temporary;
		Swap(Temporary);
	}

	void Swap(TSharedPtr& Other) noexcept
	{
		std::swap(Object, Other.Object);
		std::swap(ReferenceController, Other.ReferenceController);
	}

	[[nodiscard]] ObjectType* operator->() const
	{
		check(Object != nullptr);
		return Object;
	}

	[[nodiscard]] ObjectType& operator*() const
	{
		check(Object != nullptr);
		return *Object;
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return Object != nullptr;
	}

	explicit operator bool() const noexcept
	{
		return IsValid();
	}

	[[nodiscard]] int32 GetSharedReferenceCount() const noexcept
	{
		return ReferenceController != nullptr
			? ReferenceController->GetSharedReferenceCount()
			: 0;
	}

	[[nodisacrd]] bool IsUnique() const noexcept
	{
		return ReferenceController != nullptr && ReferenceController->IsUnique();
	}

	[[nodiscard]] TSharedRef<ObjectType, Mode> ToSharedRef() const
	{
		check(IsValid());

		if (!IsValid())
		{
			std::terminate();
		}

		return TSharedRef<ObjectType, Mode>(
			Object,
			ReferenceController,
			SharedPointerInternals::FAddSharedReferenceTag{});
	}

private:
	template<typename OtherType>
	TSharedPtr(
		const TWeakPtr<OtherType, Mode>& InWeakPtr,
		SharedPointerInternals::FFromWeakReferenceTag) noexcept
		: Object(nullptr)
		, ReferenceController(nullptr)
	{
		if (InWeakPtr.ReferenceController != nullptr &&
			InWeakPtr.ReferenceController->ConditionallyAddSharedReference())
		{
			Object = InWeakPtr.Object;
			ReferenceController = InWeakPtr.ReferenceController;
		}
	}

	TSharedPtr(
		ObjectType* InObject,
		SharedPointerInternals::FReferenceControllerBase<Mode>* InReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag) noexcept
		:Object(InObject)
		, ReferenceController(InReferenceController)
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->AddSharedReference();
		}
	}

	ObjectType* Object;
	SharedPointerInternals::FReferenceControllerBase<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedRef;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedPtr;

	template<typename OtherType, ESPMode OtherMode>
	friend class TWeakPtr;

	template<typename CastToType, typename CastFromType, ESPMode OtherMode>
	friend TSharedPtr<CastToType, OtherMode> StaticCastSharedPtr(
		const TSharedPtr<CastFromType, OtherMode>& InSharedPtr);

	template<typename CastToType, typename CastFromType, ESPMode OtherMode>
	friend TSharedPtr<CastToType, OtherMode> constCastSharedPtr(
		const TSharedPtr<CastFromType, OtherMode>& InSharedPtr);
};

template<typename ObjectType, ESPMode Mode>
class TWeakPtr
{
public:
	TWeakPtr() noexcept
		:Object(nullptr)
		, ReferenceController(nullptr)
	{
	}

	TWeakPtr(std::nullptr_t) noexcept
		:TWeakPtr()
	{
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr(const TSharedPtr<OtherType, Mode>& InSharedPtr) noexcept
		: Object(InSharedPtr.Object)
		, ReferenceController(InSharedPtr.ReferenceController)
	{
		AddWeakReference();
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr(const TSharedRef<OtherType, Mode>& InSharedRef) noexcept
		: Object(InSharedRef.Object)
		, ReferenceController(InSharedRef.ReferenceController)
	{
		AddWeakReference();
	}

	TWeakPtr(const TWeakPtr& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		AddWeakReference();
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr(const TWeakPtr<OtherType, Mode>& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		AddWeakReference();
	}

	TWeakPtr(TWeakPtr&& Other)noexcept
		: Object(std::exchange(Other.Object, nullptr))
		, ReferenceController(std::exchange(Other.ReferenceController, nullptr))
	{
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr(const TWeakPtr<OtherType, Mode>&& Other) noexcept
		: Object(Other.Object)
		, ReferenceController(Other.ReferenceController)
	{
		Other.Object = nullptr;
		Other.ReferenceController = nullptr;
	}

	TWeakPtr& operator=(const TWeakPtr& Other) noexcept
	{
		if (this != &Other)
		{
			TWeakPtr Temporary(Other);
			Swap(Temporary);
		}

		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr& operator=(const TWeakPtr<OtherType, Mode>& Other) noexcept
	{
		TWeakPtr Temporary(Other);
		Swap(Temporary);
		return *this;
	}

	TWeakPtr& operator=(TWeakPtr&& Other) noexcept
	{
		if (this != &Other)
		{
			TWeakPtr Temporary(std::move(Other));
			Swap(Temporary);
		}

		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr& operator=(const TWeakPtr<OtherType, Mode>&& Other) noexcept
	{
		TWeakPtr Temporary(std::move(Other));
		Swap(Temporary);
		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr& operator=(const TSharedPtr<OtherType, Mode>& Other) noexcept
	{
		TWeakPtr Temporary(Other);
		Swap(Temporary);
		return *this;
	}

	template<typename OtherType>
		requires std::is_convertible_v<OtherType*, ObjectType*>
	TWeakPtr& operator=(const TSharedRef<OtherType, Mode>& Other) noexcept
	{
		TWeakPtr Temporary(Other);
		Swap(Temporary);
		return *this;
	}

	TWeakPtr& operator=(std::nullptr_t) noexcept
	{
		Reset();
		return *this;
	}

	~TWeakPtr()
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->ReleaseWeakReference();
		}
	}

	void Reset() noexcept
	{
		TWeakPtr Temporary;
		Swap(Temporary);
	}

	void Swap(TWeakPtr& Other) noexcept
	{
		std::swap(Object, Other.Object);
		std::swap(ReferenceController, Other.ReferenceController);
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return Object != nullptr &&
			ReferenceController != nullptr &&
			ReferenceController->IsSharedReferenceValid();
	}

	[[nodiscard]] TSharedPtr<ObjectType, Mode> Pin() const noexcept
	{
		return TSharedPtr<ObjectType, Mode>(
			*this,
			SharedPointerInternals::FFromWeakReferenceTag{});
	}

private:
	void AddWeakReference() noexcept
	{
		if (ReferenceController != nullptr)
		{
			ReferenceController->AddWeakReference();
		}
	}

	ObjectType* Object;
	SharedPointerInternals::FReferenceControllerBase<Mode>* ReferenceController;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedRef;

	template<typename OtherType, ESPMode OtherMode>
	friend class TSharedPtr;

	template<typename OtherType, ESPMode OtherMode>
	friend class TWeakPtr;
};

template<typename ObjectType, ESPMode Mode>
TSharedPtr<ObjectType, Mode> TSharedRef<ObjectType, Mode>::ToSharedPtr() const noexcept
{
	return TSharedPtr<ObjectType, Mode>(*this);
}

template<typename ObjecType,ESPMode Mode>
TWeakPtr<ObjecType, Mode> TSharedRef<ObjecType, Mode>::ToWeakPtr() const noexcept
{
	return TWeakPtr<ObjectType, Mode>(*this);
}

template<typename ObjectType, ESPMode Mode, typename... ArgTypes>
TSharedRef<ObjectType, Mode> MakeShared(ArgTypes&&... Args)
{
	using FController = SharedPointerInternals::TInplaceReferenceController<ObjectType, Mode>;

	FController* ReferenceController =
		new FController(std::forward<ArgTypes>(Args)...);

	return TSharedRef<ObjectType, Mode>(
		ReferenceController->GetObject(),
		ReferenceController,
		SharedPointerInternals::FAdoptControllerTag{});
}

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe>
TSharedPtr<ObjectType, Mode> MakeShareable(ObjectType* InObject)
{
	return TSharedPtr<ObjectType, Mode>(InObject);
}

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe, typename DeleterType>
TSharedPtr<ObjectType, Mode> MakeShareable(ObjectType* InObject, DeleterType&& InDeleter)
{
	return TSharedPtr<ObjectType, Mode>(
		InObject,
		std::forward<DeleterType>(InDeleter));
}

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedPtr<CastToType, Mode> StaticCastSharedPtr(
	const TSharedPtr<CastFromType, Mode>& InsharedPtr)
{
	CastToType* CastObject = static_cast<CastToType*>(InsharedPtr.Object);

	return TSharedPtr<CastToType, Mode>(
		CastObject,
		InsharedPtr.ReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag{});
}

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedRef<CastToType, Mode> StaticCastSharedPtr(
	const TSharedPtr<CastFromType, Mode>& InsharedRef)
{
	CastToType* CastObject = static_cast<CastToType*>(InsharedRef.Object);

	return TSharedRef<CastToType, Mode>(
		CastObject,
		InsharedRef.ReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag{});
}

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedPtr<CastToType, Mode> ConstCastSharedPtr(
	const TSharedPtr<CastFromType, Mode>& InsharedPtr)
{
	CastToType* CastObject = const_cast<CastToType*>(InsharedPtr.Object);

	return TSharedRef<CastToType, Mode>(
		CastObject,
		InsharedPtr.ReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag{});
}

template<typename CastToType, typename CastFromType, ESPMode Mode>
TSharedRef<CastToType, Mode> ConstCastSharedPtr(
	const TSharedPtr<CastFromType, Mode>& InsharedRef)
{
	CastToType* CastObject = const_cast<CastToType*>(InsharedRef.Object);

	return TSharedRef<CastToType, Mode>(
		CastObject,
		InsharedRef.ReferenceController,
		SharedPointerInternals::FAddSharedReferenceTag{});
}

template<typename ObjectType, ESPMode Mode>
[[nodiscard]] bool operator==(const TSharedPtr<ObjectType, Mode>& Pointer, std::nullptr_t) noexcept
{
	return !Pointer.IsValid();
}

template<typename ObjectType, ESPMode Mode>
[[nodiscard]] bool operator==(std::nullptr_t, const TSharedPtr<ObjectType, Mode>& Pointer) noexcept
{
	return !Pointer.IsValid();
}

template<typename ObjectType, ESPMode Mode>
[[nodiscard]] bool operator!=(const TSharedPtr<ObjectType, Mode>& Pointer, std::nullptr_t) noexcept
{
	return Pointer.IsValid();
}

template<typename ObjectType, ESPMode Mode>
[[nodiscard]] bool operator!=(std::nullptr_t, const TSharedPtr<ObjectType, Mode>& Pointer) noexcept
{
	return Pointer.IsValid();
}