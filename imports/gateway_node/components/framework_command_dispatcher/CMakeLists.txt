set(FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../external/agile-firmware-framework")
if(NOT EXISTS "${FRAMEWORK_ROOT}/components/uhal/core/include/uhal/Status.hpp")
    message(FATAL_ERROR "Framework submodule is not initialized")
endif()

idf_component_register(
    SRCS "${FRAMEWORK_ROOT}/components/services/command_dispatcher/src/CommandDispatcher.cpp"
    INCLUDE_DIRS "${FRAMEWORK_ROOT}/components/services/command_dispatcher/include"
    REQUIRES framework_uhal_core framework_messaging framework_offline_queue framework_mqtt_contract framework_libraries
)
