set(DYNLIBUTILS_DIR "${PROJECT_SOURCE_DIR}/third_party/dynlibutils" CACHE PATH "DynLibUtils checkout")

set(DYNLIBUTILS_USE_ABI0 ON CACHE BOOL "Use the Source 2 C++ ABI" FORCE)

add_subdirectory("${DYNLIBUTILS_DIR}" dynlibutils EXCLUDE_FROM_ALL)
