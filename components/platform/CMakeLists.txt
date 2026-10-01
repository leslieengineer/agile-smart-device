idf_component_register(
    SRCS
        "esp32c6/low_level/Gpio.cpp"
        "esp32c6/low_level/SysTimer.cpp"
        "esp32c6/low_level/Watchdog.cpp"
        "esp32c6/adapters/clock/Esp32C6Clock.cpp"
        "esp32c6/adapters/gpio/Esp32C6Gpio.cpp"
        "esp32c6/adapters/watchdog/Esp32C6Watchdog.cpp"
    INCLUDE_DIRS
        "uhal/core/include"
        "uhal/interfaces/include"
        "esp32c6/adapters/clock"
        "esp32c6/adapters/gpio"
        "esp32c6/adapters/watchdog"
    PRIV_INCLUDE_DIRS "esp32c6/low_level"
    REQUIRES esp_driver_gpio esp_timer esp_system freertos
)

if(PRODUCT_PROFILE STREQUAL "matter_node")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE SMART_DEVICE_MATTER_NODE=1)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::esp_matter idf::chip)
    if(RHOPHI_CLAIM_DEV_BYPASS)
        target_compile_definitions(${COMPONENT_LIB} PRIVATE RHOPHI_CLAIM_DEV_BYPASS=1)
    endif()
endif()
