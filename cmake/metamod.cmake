set(METAMOD_DIR "${PROJECT_SOURCE_DIR}/third_party/metamod" CACHE PATH "Metamod:Source checkout")

if(NOT EXISTS "${METAMOD_DIR}/core/ISmmPlugin.h")
	message(FATAL_ERROR "Missing Metamod")
endif()

# Metamod supplies KHook at runtime; only its headers are needed here.
add_library(clientcvarvalue_metamod INTERFACE)
target_compile_definitions(clientcvarvalue_metamod INTERFACE META_IS_SOURCE2)
target_include_directories(clientcvarvalue_metamod INTERFACE
	"${METAMOD_DIR}/core"
	"${METAMOD_DIR}/third_party/khook/include"
)
