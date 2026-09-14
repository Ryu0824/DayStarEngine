find_package(Threads REQUIRED)
set(MemoryCoreRoot "${PROJECT_SOURCE_DIR}/Source/Runtime/Core")

function(daystar_configure_memory_program TargetName)
	target_compile_features(${TargetName} PRIVATE cxx_std_20)
	target_link_libraries(${TargetName} PRIVATE Threads::Threads)
	if(WIN32)
		target_compile_definitions(${TargetName} PRIVATE UNICODE _UNICODE)
	endif()
endfunction()

function(daystar_add_memory_case TestName TargetName)
	add_test(NAME ${TestName} COMMAND ${TargetName} ${ARGN})
	set_tests_properties(${TestName} PROPERTIES LABELS "MemoryTests" TIMEOUT 60)
endfunction()

function(daystar_add_memory_native TargetName Source)
	add_executable(${TargetName} ${Source})
	target_link_libraries(${TargetName} PRIVATE Core)
	daystar_configure_memory_program(${TargetName})
endfunction()

function(daystar_add_memory_isolated TargetName)
	add_executable(${TargetName} ${ARGN})
	target_include_directories(${TargetName} PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/HostApi"
		"${MemoryCoreRoot}/Public"
		"${MemoryCoreRoot}/Private"
		"${CMAKE_CURRENT_SOURCE_DIR}"
	)
	daystar_configure_memory_program(${TargetName})
endfunction()

function(daystar_add_memory_exit_case TestName TargetName Mode ExpectedCode)
	add_test(NAME ${TestName}
		COMMAND "${CMAKE_COMMAND}"
			"-DPROGRAM=$<TARGET_FILE:${TargetName}>"
			"-DMODE=${Mode}"
			"-DEXPECTED_CODE=${ExpectedCode}"
			-P "${CMAKE_CURRENT_SOURCE_DIR}/CheckExitCode.cmake"
	)
	set_tests_properties(${TestName} PROPERTIES LABELS "MemoryTests;MemoryDeathTests" TIMEOUT 30)
endfunction()

daystar_add_memory_native(MemorySystemTests SystemMemoryTests.cpp)
daystar_add_memory_case(Memory.SystemContract MemorySystemTests)
daystar_add_memory_native(MemoryLifetimeTests MemoryLifetimeTests.cpp)
daystar_add_memory_case(Memory.ProcessLifetime MemoryLifetimeTests)
daystar_add_memory_native(MemoryBinnedTests BinnedTests.cpp)
daystar_add_memory_case(Memory.BinnedContract MemoryBinnedTests)
if(WIN32)
	daystar_add_memory_native(MemoryWindowsTests WindowsIntegrationTests.cpp)
	target_link_libraries(MemoryWindowsTests PRIVATE kernel32)
	daystar_add_memory_case(Memory.WindowsPages MemoryWindowsTests)
endif()

option(DAYSTAR_BUILD_MEMORY_FAULT_TESTS "Build isolated memory fault-injection tests" ON)
if(DAYSTAR_BUILD_MEMORY_FAULT_TESTS)
	daystar_add_memory_isolated(MemorySystemInjectedTests
		"${MemoryCoreRoot}/Private/HAL/FMalloc.cpp"
		"${MemoryCoreRoot}/Private/HAL/FMemory.cpp"
		"${MemoryCoreRoot}/Private/HAL/MallocSystem.cpp"
		HostEmergency.cpp FakeSystemHeap.cpp SystemMemoryTests.cpp
	)
	target_compile_definitions(MemorySystemInjectedTests PRIVATE DAYSTAR_TEST_SYSTEM_HEAP)
	daystar_add_memory_case(Memory.SystemInjectedContract MemorySystemInjectedTests)
	daystar_add_memory_exit_case(Memory.SystemInvalidAlignment MemorySystemInjectedTests invalid_alignment 91)
	daystar_add_memory_exit_case(Memory.SystemFatalOverflow MemorySystemInjectedTests fatal-overflow 94)
	daystar_add_memory_exit_case(Memory.SystemFatalMalloc MemorySystemInjectedTests fatal-oom 94)
	daystar_add_memory_exit_case(Memory.SystemFatalRealloc MemorySystemInjectedTests fatal-realloc 94)
	
	daystar_add_memory_isolated(MemoryBinnedHostTests
		"${MemoryCoreRoot}/Private/HAL/FMalloc.cpp"
		"${MemoryCoreRoot}/Private/HAL/PageRegion.cpp"
		"${MemoryCoreRoot}/Private/HAL/MallocBinned.cpp"
		BinnedHostPlatform.cpp BinnedTests.cpp
	)
	target_compile_definitions(MemoryBinnedHostTests PRIVATE DAYSTAR_TEST_BINNED_PLATFORM)
	daystar_add_memory_case(Memory.BinnedSimulated4K MemoryBinnedHostTests normal)
	daystar_add_memory_case(Memory.BinnedSimulated16K MemoryBinnedHostTests page16K)
	daystar_add_memory_exit_case(Memory.BinnedInvalidAlignment MemoryBinnedHostTests badalign 91)
	daystar_add_memory_exit_case(Memory.BinnedInteriorPointer MemoryBinnedHostTests interior 91)
	daystar_add_memory_exit_case(Memory.BinnedDoubleFree MemoryBinnedHostTests doublefree 91)
	daystar_add_memory_exit_case(Memory.BinnedForeignPointer MemoryBinnedHostTests foreign 91)
	daystar_add_memory_exit_case(Memory.BinnedLiveDestruction MemoryBinnedHostTests live_destroy 92)

	daystar_add_memory_isolated(MemoryPlatformHostTests
		"${MemoryCoreRoot}/Private/HAL/PageRegion.cpp"
		"${MemoryCoreRoot}/Private/HAL/Windows/WindowsPlatformMemory.cpp"
		HostContractTests.cpp
	)
	target_include_directories(MemoryPlatformHostTests BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/FakeWinApi")
	target_compile_definitions(MemoryPlatformHostTests PRIVATE DAYSTAR_TEST_WINAPI)
	daystar_add_memory_case(Memory.PlatformSimulated4K MemoryPlatformHostTests 4096)
	daystar_add_memory_case(Memory.PlatformSimulated16K MemoryPlatformHostTests 16384)
	daystar_add_memory_exit_case(Memory.PlatformFatalRelease MemoryPlatformHostTests fatal-release 86)
	daystar_add_memory_exit_case(Memory.PlatformInvalidConstants MemoryPlatformHostTests bad-system-info 86)
endif()