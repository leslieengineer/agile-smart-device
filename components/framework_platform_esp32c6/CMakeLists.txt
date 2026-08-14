set(FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../external/agile-firmware-framework")
if(NOT EXISTS "${FRAMEWORK_ROOT}/components/uhal/core/include/uhal/Status.hpp")
    message(FATAL_ERROR "Framework submodule is not initialized. Run: git submodule update --init --recursive")
endif()

idf_component_register(
    SRCS
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/low_level/system/Gpio.cpp"
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/low_level/system/SysTimer.cpp"
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/adapters/gpio/Esp32C6Gpio.cpp"
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/adapters/clock/Esp32C6Clock.cpp"
    INCLUDE_DIRS
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/low_level/system"
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/adapters/gpio"
        "${FRAMEWORK_ROOT}/components/platform/esp32c6/esp_idf/adapters/clock"
    REQUIRES framework_uhal_interfaces esp_driver_gpio esp_timer freertos
)
