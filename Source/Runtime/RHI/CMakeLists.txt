set(RHI_SOURCES
	Private/DynamicRHI.cpp
	Private/RHIResources.cpp
)

add_library(RHI SHARED ${RHI_SOURCES})

target_include_directories(RHI
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(RHI PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(RHI PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(RHI PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(RHI PUBLIC Core)

target_compile_definitions(RHI PRIVATE RHI_EXPORT=1)