set(PRODUCT_SRCS
    "src/composition/SmartDevice.cpp"
    "src/application/SmartDeviceApplication.cpp"
    "src/runtime/SwitchRuntime.cpp"
    "src/adapters/NvsBinaryStateStore.cpp"
)

set(PRODUCT_PRIVATE_INCLUDE_DIRS
    "src/application"
    "src/runtime"
    "src/adapters"
)

# Đã thay 4 dòng framework_* cũ bằng: platform và middleware
set(PRODUCT_REQUIRES
    platform
    middleware
    board_esp32c6
    nvs_flash
    esp_timer
    freertos
)

if(PRODUCT_PROFILE STREQUAL "matter_node")
    list(APPEND PRODUCT_SRCS
        "src/connectivity/RhophiClaimProtocol.cpp"
        "src/connectivity/MatterNode.cpp"
        "src/connectivity/RhophiClaimPlatform.cpp"
        "src/connectivity/RhophiClaimGatt.cpp"
        "src/adapters/NvsClaimStateStore.cpp"
    )
    list(APPEND PRODUCT_PRIVATE_INCLUDE_DIRS "src/connectivity")
    list(APPEND PRODUCT_REQUIRES esp_matter chip openthread led_strip mbedtls)
endif()

idf_component_register(
    SRCS ${PRODUCT_SRCS}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS ${PRODUCT_PRIVATE_INCLUDE_DIRS}
    REQUIRES ${PRODUCT_REQUIRES}
)

if(PRODUCT_PROFILE STREQUAL "matter_node")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE SMART_DEVICE_MATTER_NODE=1)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::esp_matter idf::chip)
    if(RHOPHI_CLAIM_DEV_BYPASS)
        target_compile_definitions(${COMPONENT_LIB} PRIVATE RHOPHI_CLAIM_DEV_BYPASS=1)
    endif()
endif()