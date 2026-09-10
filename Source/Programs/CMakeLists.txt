include(FetchContent)

# Match the engine's default MSVC DLL runtime (/MD, /MDd).
if(MSVC)
	set(gtest_force_shared_crt ON CACHE BOOL "Use the shared CRT for GoogleTest" FORCE)
endif()

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

function(daystar_add_test_target TargetName)
	cmake_parse_arguments(TEST "" "" "SOURCES;MODULES" ${ARGN})
	if(TEST_UNPARSED_ARGUMENTS OR NOT TEST_SOURCES OR NOT TEST_MODULES)
		message(FATAL_ERROR "Invalid test target definition: ${TargetName}")
	endif()

	add_executable(${TargetName} ${TEST_SOURCES})
	target_link_libraries(${TargetName} PRIVATE ${TEST_MODULES} GTest::gtest_main)

	#Source scanning does not launch an excutable during configure/build
	#Revisit discovery if typed or parameterized tests are introduced.
	gtest_add_tests(
		TARGET ${TargetName}
		SOURCES ${TEST_SOURCES}
		TEST_PREFIX "${TargetName}."
		TEST_LIST RegisteredTests
	)
	if(NOT RegisteredTests)
		message(FATAL_ERROR "No tests registered for ${TargetName}")
	endif()
	set_tests_properties(${RegisteredTests} PROPERTIES
		LABELS "${TargetName}"
		TIMEOUT 30
	)
endfunction()

daystar_add_test_target(CoreTests
	MODULES Core
	SOURCES
		CoreTests/AssertionTest.cpp
		CoreTests/CoreContainerTest.cpp
		CoreTests/CoreDelegateTest.cpp
		CoreTests/CoreMemoryTest.cpp
		CoreTests/CoreSharedPointerTest.cpp
		CoreTests/MathTest.cpp
		CoreTests/ModuleManagerTest.cpp
		CoreTests/OutputDeviceTest.cpp
)

daystar_add_test_target(CoreUObjectTests
	MODULES Core CoreUObject
	SOURCES
		CoreUObjectTests/GarbageCollectionTest.cpp
		CoreUObjectTests/UObjectCoreTest.cpp
)

daystar_add_test_target(ApplicationCoreTests
	MODULES Core ApplicationCore
	SOURCES ApplicationCoreTests/ApplicationCoreTests.cpp
)

daystar_add_test_target(RHITests
	MODULES Core RHI
	SOURCES RHITests/DynamicRHITest.cpp
)

daystar_add_test_target(RenderCoreTests
	MODULES Core RHI RenderCore
	SOURCES RenderCoreTests/RenderCoreTest.cpp
)

daystar_add_test_target(LaunchTests
	MODULES Core RenderCore Launch
	SOURCES LaunchTests/EngineLoopTest.cpp
)

daystar_add_test_target(EngineTests
	MODULES Core CoreUObject Engine
	SOURCES EngineTests/GameFrameworkTest.cpp
)