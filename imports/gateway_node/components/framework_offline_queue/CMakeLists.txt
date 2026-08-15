set(FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../external/agile-firmware-framework")
if(NOT EXISTS "${FRAMEWORK_ROOT}/components/uhal/core/include/uhal/Status.hpp")
    message(FATAL_ERROR "Framework submodule is not initialized")
endif()

idf_component_register(
    INCLUDE_DIRS "${FRAMEWORK_ROOT}/components/services/offline_queue/include"
    REQUIRES framework_uhal_core framework_messaging framework_services_types framework_libraries
)
