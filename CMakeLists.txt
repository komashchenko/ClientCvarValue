cmake_minimum_required(VERSION 3.25)

project(ClientCvarValue
	VERSION 1.1.0
	DESCRIPTION "API for query cvar value of players"
	HOMEPAGE_URL "https://github.com/komashchenko/ClientCvarValue"
	LANGUAGES C CXX
)

include(cmake/platform/shared.cmake)

include(cmake/sourcesdk.cmake)
include(cmake/dynlibutils.cmake)
include(cmake/metamod.cmake)

add_library(clientcvarvalue SHARED
	src/clientcvarvalue.cpp
)

set_target_properties(clientcvarvalue PROPERTIES
	PREFIX ""
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

target_include_directories(clientcvarvalue PRIVATE src public)
target_compile_definitions(clientcvarvalue PRIVATE
	PLUGIN_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(clientcvarvalue PRIVATE
	sourcesdk::sourcesdk
	cpp-memory_utils
	clientcvarvalue_metamod
	clientcvarvalue_platform
)

include(cmake/package.cmake)
