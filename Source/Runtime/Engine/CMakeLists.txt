set(ENGINE_SOURCES
	Private/Components/ActorComponent.cpp
	Private/GameFramework/Actor.cpp
	Private/Engine/World.cpp
)

add_library(Engine SHARED ${ENGINE_SOURCES})

target_include_directories(Engine
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(Engine PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(Engine PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(Engine PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(Engine PUBLIC Core CoreUObject)

target_compile_definitions(Engine PRIVATE ENGINE_EXPORT=1)
target_compile_definitions(Engine PRIVATE UNICODE _UNICODE)