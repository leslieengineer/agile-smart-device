set(FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../external/agile-firmware-framework")
if(NOT EXISTS "${FRAMEWORK_ROOT}/components/uhal/core/include/uhal/Status.hpp")
    message(FATAL_ERROR "Framework submodule is not initialized. Run: git submodule update --init --recursive")
endif()

idf_component_register(
    SRCS "${FRAMEWORK_ROOT}/components/services/binary_switch/src/BinarySwitchService.cpp"
    INCLUDE_DIRS "${FRAMEWORK_ROOT}/components/services/binary_switch/include"
    REQUIRES framework_uhal_core framework_uhal_interfaces
)
