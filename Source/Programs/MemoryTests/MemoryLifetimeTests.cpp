#include "HAL/FMemory.h"
#include "TestCheck.h"
struct FGlobalOwner
{
	void* Data = FMemory::Malloc(128, 64);
	~FGlobalOwner() { FMemory::Free(Data); std::puts("PASS allocation before main and free during static destruction"); }
} GlobalOwner;
int main() { REQUIRE(GlobalOwner.Data != nullptr); }