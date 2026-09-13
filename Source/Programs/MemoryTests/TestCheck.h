#pragma once
#include <cstdio>
#include <cstdlib>
#define REQUIRE(Expression) do { if(!(Expression)){\
	std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__,__LINE__, #Expression); \
	std::abort();}} while(false)