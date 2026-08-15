set(FRAMEWORK_UHAL_CORE_INCLUDE
 "${CMAKE_CURRENT_LIST_DIR}/../../../../external/agile-firmware-framework/components/uhal/core/include"
)

if(NOT EXISTS "${FRAMEWORK_UHAL_CORE_INCLUDE}/uhal/Status.hpp")
 message(FATAL_ERROR
 "Framework submodule is not initialized. Run: git submodule update --init --recursive"
 )
endif()

idf_component_register(
 INCLUDE_DIRS "${FRAMEWORK_UHAL_CORE_INCLUDE}"
)
