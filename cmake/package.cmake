set(PLUGIN_NAME clientcvarvalue)
set(PLUGIN_BINARY_DIR "addons/${PLUGIN_NAME}")
set(PACKAGE_ROOT "${PROJECT_BINARY_DIR}/package")

set_target_properties(clientcvarvalue PROPERTIES
	OUTPUT_NAME "${PLUGIN_NAME}"
	RUNTIME_OUTPUT_DIRECTORY "${PACKAGE_ROOT}/${PLUGIN_BINARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PACKAGE_ROOT}/${PLUGIN_BINARY_DIR}"
	PDB_OUTPUT_DIRECTORY "${PACKAGE_ROOT}/${PLUGIN_BINARY_DIR}"
)

if(MSVC)
	# Debug keeps incremental linking; place its database outside the package.
	target_link_options(clientcvarvalue PRIVATE
		"$<$<CONFIG:Debug>:/ILK:${PROJECT_BINARY_DIR}/${PLUGIN_NAME}.ilk>")
endif()

file(MAKE_DIRECTORY "${PACKAGE_ROOT}/addons/metamod")
file(CONFIGURE
	OUTPUT "${PACKAGE_ROOT}/addons/metamod/${PLUGIN_NAME}.vdf"
	CONTENT [=[
"Metamod Plugin"
{
	"alias"	"@PLUGIN_NAME@"
	"file"	"@PLUGIN_BINARY_DIR@/@PLUGIN_NAME@"
}
]=]
	@ONLY
	NEWLINE_STYLE CRLF
)
