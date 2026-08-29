#pragma once

template<typename T>
class TRefCountPtr
{
public:
	TRefCountPtr() :Reference(nullptr) {}

	TRefCountPtr(T* InReference) : Reference(InReference)
	{
		if (Reference) { Reference->AddRef(); }
	}

	TRefCountPtr(const TRefCountPtr& Copy) : TRefCountPtr(Copy.Reference)
	{
		if (Reference) { Reference->AddRef(); }
	}

	TRefCountPtr(TRefCountPtr&& Move) noexcept : Reference(Move.Reference)
	{
		Move.Reference = nullptr;
	}

	~TRefCountPtr()
	{
		if (Reference) { Reference->Release(); }
	}

	T* operator->() const { return Reference; }
	T* Get() const { return Reference; }
	bool IsValid() const { return Reference != nullptr; }

	TRefCountPtr* operator=(T* InReference)
	{
		if (Reference != InReference)
		{
			T* OldReference = Reference;
			Reference = InReference;
			if (Reference) { Reference->AddRef(); }
			if (OldReference) { OldReference->Release(); }
		}
		return *this;
	}

private:
	T* Reference;
};