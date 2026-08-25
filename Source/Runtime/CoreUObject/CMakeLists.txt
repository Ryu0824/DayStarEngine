set(COREUOBJECT_SOURCES
	Private/UObject/Object.cpp
	Private/UObject/Class.cpp
)

add_library(CoreUObject SHARED ${COREUOBJECT_SOURCES})

target_include_directories(CoreUObject
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Private>
)

if(WIN32)
	target_compile_definitions(CoreUObject PUBLIC PLATFORM_WINDOWS=1)
elseif(APPLE)
	target_compile_definitions(CoreUObject PUBLIC PLATFORM_MAC=1)
elseif(UNIX)
	target_compile_definitions(CoreUObject PUBLIC PLATFORM_LINUX=1)
endif()

target_link_libraries(CoreUObject PUBLIC Core)

target_compile_definitions(CoreUObject PRIVATE COREUOBJECT_EXPORT=1)
