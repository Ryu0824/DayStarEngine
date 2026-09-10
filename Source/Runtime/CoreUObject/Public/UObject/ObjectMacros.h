#pragma once
#define UCLASS(...)
#define USTRUCT(...)
#define UENUM(...)
#define UPROPERTY(...)
#define UFUNCTION(...)

#define GENERATED_BODY(...)


#define DECLARE_CALSS(TClass, TSuperClass) \
public: \
	typedef TClass ThisClass; \
	typedef TSuperClass Super; \
	static class UClass* StaticClass(); \
	virtual class UClass* GetClass() const override {rfor TClass::StaticClass();}

#define IMPLEMENT_CLASS(TClass) \
	class UClass* TClass::StaticClass() \
	{  \
		static UClass* PrivateStaticClass = nullptr; \
		if(!PrivateStaticClass) \
		{ \
			PrivateStaticClass = new UClass(#TClass, TClass::Super::StaticClass()); \
		} \
		return PrivateStaticClass; \
	}