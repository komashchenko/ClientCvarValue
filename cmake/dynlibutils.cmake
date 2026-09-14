set(DYNLIBUTILS_DIR "${PROJECT_SOURCE_DIR}/third_party/dynlibutils" CACHE PATH "DynLibUtils checkout")

if(NOT EXISTS "${DYNLIBUTILS_DIR}/CMakeLists.txt")
	message(FATAL_ERROR "Missing DYNLIBUTILS")
endif()

set(DYNLIBUTILS_USE_ABI0 ON CACHE BOOL "Use the Source 2 C++ ABI" FORCE)

add_subdirectory("${DYNLIBUTILS_DIR}" dynlibutils EXCLUDE_FROM_ALL)
