#pragma once
#include "CoreTypes.h"
#include "HAL/FMemory.h"
#include <atomic>

enum class ESPMode
{
	NotThreadSafe,
	ThreadSafe
};

namespace SharedPointerInternals
{
	struct FAdoptControllerTag
	{
	};

	struct FAddSharedReferenceTag
	{
	};

	struct FFromWeakReferenceTag
	{
	};

	template<ESPMode Mode>
	class FReferenceControllerBase
	{
	private:
		using FReferenceCount = std::conditional_t<
			Mode = ESPMode::ThreadSafe,
			std::atomic<int32>,
			int32>;

	public:
		FReferenceControllerBase() noexcept
			: SharedReferenceCount(1)
			, WeakReferenceCount(1)
		{
		}

		FReferenceControllerBase(const FReferenceControllerBase&) = delete;
		FReferenceControllerBase& operator=(const FReferenceControllerBase&) = delete;

		void AddSharedReference() noexcept
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				const int32 PreviousCount = SharedReferenceCount.fetch_add(1, std::memory_order_relaxed);

				if (PreviousCount <= 0)
				{
					std::terminate();
				}
			}
			else
			{
				if (SharedReferenceCount <= 0)
				{
					std::terminate();
				}

				++SharedReferenceCount;
			}
		}

		[[nodiscard]] bool ConditionallyAddSharedReference()noexcept
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				int32 CurrentCount = SharedReferenceCount.load(std::memory_order_acquire);

				while (CurrentCount > 0)
				{
					if (SharedReferenceCount.compare_exchange_weak(
						CurrentCount,
						CurrentCount + 1,
						std::memory_order_acquire,
						std::memory_order_relaxed))
					{
						return true;
					}
				}

				return false;
			}
			else
			{
				if (SharedReferenceCount == 0)
				{
					return false;
				}

				++SharedReferenceCount;
				return true;
			}
		}

		void ReleaseSharedReference() noexcept
		{
			bool bWasLastSharedReference = false;

			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				const int32 PreviousCount = SharedReferenceCount.fetch_sub(1, std::memory_order_acq_rel);

				if (PreviousCount <= 0)
				{
					std::terminate();
				}

				bWasLastSharedReference = PreviousCount == 1;
			}
			else
			{
				if (SharedReferenceCount <= 0)
				{
					std::terminate();
				}

				bWasLastSharedReference = --SharedReferenceCount == 0;
			}

			if (bWasLastSharedReference)
			{
				DestroyObject();
				ReleaseWeakReference();
			}
		}

		void AddWeakReference() noexcept
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				const int32 PreviousCount = WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);

				if (PreviousCount <= 0)
				{
					std::terminate();
				}
			}
			else
			{
				if (WeakReferenceCount <= 0)
				{
					std::terminate();
				}

				++AddWeakReference;
			}
		}

		void ReleaseWeakReference() noexcept
		{
			bool bWasLastWeakReference = false;

			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				const int32 PreviousCount = WeakReferenceCount.fetch_sub(1, std::memory_order_acq_rel);

				if (PreviousCount <= 0)
				{
					std::terminate();
				}

				bWasLastWeakReference = PreviousCount == 1;
			}
			else
			{
				if (WeakReferenceCount <= 0)
				{
					std::terminate();
				}

				bWasLastWeakReference = --WeakReferenceCount == 0;
			}

			if (bWasLastWeakReference)
			{
				delete this;
			}
		}

		[[nodiscard]] bool IsSharedReferenceValid() const noexcept
		{
			return GetSharedReferenceCount() > 0;
		}

		[[nodiscard]] int32 GetSharedReferenceCount() const noexcept
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				return SharedReferenceCount.load(std::memory_order_acquire);
			}
			else
			{
				return SharedReferenceCount;
			}
		}

		[[nodiscard]] bool IsUnique() const noexcept
		{
			return GetSharedReferenceCount() == 1;
		}

	protected:
		virtual ~FReferenceControllerBase() = default;
		virtual void DestroyObject() noexcept = 0;

	private:
		FReferenceCount SharedReferenceCount;
		FReferenceCount WeakReferenceCount;

	};

	template<typename ObjectType, typename DeleterType, ESPMode Mode>
	class TReferenceControllerWithDeleter final : public FReferenceControllerBase<Mode>
	{
	public:
		template<typename InDeleterType>
		TReferenceControllerWithDeleter(ObjectType* InObject, InDeleterType&& InDeleter)
			: Object(InObject)
			, Deleter(std::forward<InDeleterType>(InDeleter))
		{
		}

	protected:
		virtual void DestroyObject() noexcept override
		{
			ObjectType* ObjectToDestroy = std::exchange(Object, nullptr);

			if (ObjectToDestroy != nullptr)
			{
				std::invoke(Deleter, ObjectToDestroy);
			}
		}

	private:
		ObjectType* Object;
		[[no_unique_address]] DeleterType Deleter;
	};

	template<typename ObjectType, ESPMode Mode>
	class TInplaceReferenceController final : public FReferenceControllerBase<Mode>
	{
	public:
		template<typename... ArgTypes>
		explicit TInplaceReferenceController(ArgTypes&&... Args)
		{
			::new (static_cast<void*>(Storage)) ObjectType(std::forward<ArgTypes>(Args)...);
			bObjectAlive = true;
		}

		[[nodiscard]] ObjectType* GetObject() noexcept
		{
			return std::launder(reinterpret_cast<ObjectType*>(Storage));
		}

	protected:
		virtual void DestroyObject() noexcept override
		{
			if (bObjectAlive)
			{
				GetObject()->~ObjectType();
				bObjectAlive = false;
			}
		}

	private:
		alignas(ObjectType) std::byte Storage[sizeof(ObjectType)];
		bool bObjectAlive = false;
	};

	template<typename ObjectType, ESPMode Mode, typename DeleterType>
	[[nodiscard]] FReferenceControllerBase<Mode>* CreateReferenceController(
		ObjectType* InObject,
		DeleterType&& InDeleter)

	{
		using FStoreDeleter = std::decay_t<DeleterType>;
		using FController = TReferenceControllerWithDeleter<ObjectType, FStoreDeleter, Mode>;

		try
		{
			return new FController(InObject, std::forward<DeleterType>(InDeleter));
		}
		catch (...)
		{
			std::invoke(InDeleter, InObject);
			throw;
		}
	}
}

