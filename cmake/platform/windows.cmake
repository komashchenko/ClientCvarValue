set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Required for correct interaction with the game's protobuf messages in Debug builds.
# Apply before add_subdirectory() so the plugin and dependencies share the STL ABI.
add_compile_definitions("$<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>")

target_compile_options(clientcvarvalue_platform INTERFACE /W3)

# Remove unused code while keeping identical functions separate for debugging.
target_link_options(clientcvarvalue_platform INTERFACE
	"$<$<CONFIG:RelWithDebInfo>:/OPT:REF;/OPT:NOICF;/INCREMENTAL:NO>"
)
