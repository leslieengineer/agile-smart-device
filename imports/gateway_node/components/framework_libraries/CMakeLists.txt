set(FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../external/agile-firmware-framework")
if(NOT EXISTS "${FRAMEWORK_ROOT}/components/uhal/core/include/uhal/Status.hpp")
    message(FATAL_ERROR "Framework submodule is not initialized")
endif()

idf_component_register(INCLUDE_DIRS "${FRAMEWORK_ROOT}/components/libraries/include" "${FRAMEWORK_ROOT}/components/libraries/ring_buffer/include" "${FRAMEWORK_ROOT}/components/libraries/retry/include" "${FRAMEWORK_ROOT}/components/libraries/serialization/include")
