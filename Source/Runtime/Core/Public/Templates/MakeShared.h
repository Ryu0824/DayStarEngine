#pragma once
#include "Templates/SharedPointer.h"

template<typename ObjectType, ESPMode Mode = ESPMode::NotThreadSafe, typename... Args>
TSharedRef<ObjectType, Mode> MakeShared(Args&& ...InArgs)
{
	ObjectType* NewObject = new ObjectType(std::forward<Args>(InArgs)...);
	return TSharedRef<ObjectType, Mode>(NewObject);
}