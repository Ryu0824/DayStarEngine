set(CORE_SOURCES
	Private/HAL/FMemory.cpp
	Private/Containers/UnrealString.cpp
	Private/HAL/PlatformMisc.cpp
	Private/Logging/LogMacros.cpp
	Private/MISC/AssertionMacros.cpp
)

add_library(Core SHARED ${CORE_SOURCES})

target_include_directories(Core
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(Core PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(Core PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(Core PUBLIC PLATFORM_LINUX=1)
endif()

target_compile_definitions(Core PRIVATE CORE_EXPORT=1)
