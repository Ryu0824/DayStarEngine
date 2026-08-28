include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(MathTest
	CoreTests/MathTest.cpp
)

target_link_libraries(MathTest PRIVATE Core CoreUObject gtest_main)