idf_component_register(
    SRCS
        "src/composition/SmartDevice.cpp"
        "src/application/SmartDeviceApplication.cpp"
        "src/runtime/SwitchRuntime.cpp"
        "src/adapters/NvsBinaryStateStore.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src/runtime" "src/adapters"
    REQUIRES
        framework_uhal_core
        framework_uhal_interfaces
        framework_button
        framework_binary_switch
        board_esp32c6
        nvs_flash
        freertos
)
