include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(DynamicRHITest
	RHITests/DynamicRHITest.cpp
)

target_link_libraries(DynamicRHITest PRIVATE Core RHI gtest_main)