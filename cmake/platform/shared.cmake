set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(clientcvarvalue_platform INTERFACE)

if(WIN32)
	include("${CMAKE_CURRENT_LIST_DIR}/windows.cmake")
else()
	include("${CMAKE_CURRENT_LIST_DIR}/linux.cmake")
endif()
