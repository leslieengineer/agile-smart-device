idf_component_register(
    SRCS
        "src/SmartDevice.cpp"
        "src/SwitchController.cpp"
        "src/SwitchRuntime.cpp"
        "src/adapters/NvsRelayStateRepository.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES
        framework_uhal_core
        framework_uhal_interfaces
        framework_button
        board_esp32c6
        nvs_flash
        freertos
)
