#include "HAL/PlatformMemory.h"
#include <utility>

FPageRegion::FPageRegion(FPageRegion&& Other)noexcept
	:Base(std::exchange(Other.Base, nullptr))
	, Size(std::exchange(Other.Size, 0))
	, RequestedSize(std::exchange(Other.RequestedSize, 0))
{
}

FPageRegion::~FPageRegion() noexcept
{
	const auto Result = FPlatformMemory::ReleasePages(*this);
	if (!Result.Succeeded())
	{
		FPlatformMemory::EmergencyTerminate(Result);
	}
}