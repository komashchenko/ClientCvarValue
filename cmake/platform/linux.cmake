set(CMAKE_SKIP_RPATH ON)

target_compile_options(clientcvarvalue_platform INTERFACE
	-fno-exceptions
	-fno-rtti
	-fno-omit-frame-pointer
)
target_link_options(clientcvarvalue_platform INTERFACE
	LINKER:--no-undefined
	-static-libstdc++
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(clientcvarvalue_platform INTERFACE -fno-gnu-unique)
	target_link_options(clientcvarvalue_platform INTERFACE -static-libgcc)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_link_libraries(clientcvarvalue_platform INTERFACE gcc_eh)
endif()
