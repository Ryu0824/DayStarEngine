set(RENDERCORE_SOURCES
	Private/RenderCommandQueue.cpp
)

add_library(RenderCore SHARED ${RENDERCORE_SOURCES})

target_include_directories(RenderCore
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(RenderCore PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(RenderCore PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(RenderCore PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(RenderCore PUBLIC Core)

target_compile_definitions(RenderCore PRIVATE RENDERCORE_EXPORT=1)