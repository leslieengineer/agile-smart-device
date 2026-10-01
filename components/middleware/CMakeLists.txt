idf_component_register(
    SRCS
        "libraries/button/src/ButtonInput.cpp"
        "services/binary_switch/src/BinarySwitchService.cpp"
    INCLUDE_DIRS
        "libraries/button/include"
        "services/binary_switch/include"
    REQUIRES platform
)
