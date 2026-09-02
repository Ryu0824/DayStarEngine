set(APPLICATIONCORE_SOURECE
	Private/GenericPlatform/GenericApplicationMessageHandler.cpp
	Private/GenericPlatform/GenericApplication.cpp
	Private/GenericPlatform/GenericWindow.cpp
)

add_library(ApplicationCore SHARED ${APPLICATIONCORE_SOURECE})

target_include_directories(ApplicationCore
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(ApplicationCore PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(ApplicationCore PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(ApplicationCore PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(ApplicationCore PUBLIC Core)

target_compile_definitions(ApplicationCore PRIVATE APPLICATIONCORE_EXPORT=1)
