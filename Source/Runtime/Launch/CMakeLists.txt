set(LAUNCH_SOURCES
	Private/EngineLoop.cpp
	Private/Launch.cpp
	Private/Windows/LaunchWindows.cpp
)

add_library(Launch STATIC ${LAUNCH_SOURCES})

target_include_directories(Launch
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(Launch PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(Launch PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(Launch PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(Launch PUBLIC Core ApplicationCore RenderCore)

target_compile_definitions(Launch PRIVATE LAUNCH_EXPORT=1)
target_compile_definitions(Launch PRIVATE UNICODE _UNICODE)