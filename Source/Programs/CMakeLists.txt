include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(AssertionTest
	CoreTests/AssertionTest.cpp
)

target_link_libraries(AssertionTest PRIVATE Core CoreUObject gtest_main)