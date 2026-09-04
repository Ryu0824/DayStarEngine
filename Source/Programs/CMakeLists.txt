include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(EngineLoopTest
	LaunchTests/EngineLoopTest.cpp
)

target_link_libraries(EngineLoopTest PRIVATE Core RenderCore Launch gtest_main)