#include "UObject/Object.h"
#include "UObject/Class.h"

UObject::UObject()
	:ClassPrivate(nullptr)
	, ObjectFlags(RF_NoFlags)
{

}

UObject::~UObject()
{

}

UClass* UObject::StaticClass()
{
	static UClass* ObjectClass = new UClass(TEXT("UObject"), nullptr);
	return ObjectClass;
}