cmake_minimum_required(VERSION 3.16)

set(PRODUCT_PROFILE "local_switch" CACHE STRING "Firmware product profile")
set_property(CACHE PRODUCT_PROFILE PROPERTY STRINGS local_switch gateway_node matter_node)

list(APPEND EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/imports/common/components"
    "${CMAKE_CURRENT_LIST_DIR}/imports/local_switch/components"
)

if(PRODUCT_PROFILE STREQUAL "gateway_node")
    list(APPEND EXTRA_COMPONENT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/imports/gateway_node/components"
    )
elseif(PRODUCT_PROFILE STREQUAL "matter_node")
    if(NOT DEFINED ENV{ESP_MATTER_PATH})
        message(FATAL_ERROR "matter_node requires ESP_MATTER_PATH")
    endif()
    set(ESP_MATTER_PATH "$ENV{ESP_MATTER_PATH}")
    set(MATTER_SDK_PATH "${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip")
    if(NOT EXISTS "${MATTER_SDK_PATH}/config/esp32/components/chip/CMakeLists.txt")
        message(FATAL_ERROR "ESP_MATTER_PATH is incomplete; initialize connectedhomeip submodules")
    endif()
    list(APPEND EXTRA_COMPONENT_DIRS
        "${MATTER_SDK_PATH}/config/esp32/components"
        "${ESP_MATTER_PATH}/components"
    )
elseif(NOT PRODUCT_PROFILE STREQUAL "local_switch")
    message(FATAL_ERROR "Unsupported PRODUCT_PROFILE: ${PRODUCT_PROFILE}")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(agile_smart_device)

if(PRODUCT_PROFILE STREQUAL "matter_node")
    idf_build_set_property(CXX_COMPILE_OPTIONS
        "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
    idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
    idf_build_set_property(COMPILE_OPTIONS
        "-Wno-format-nonliteral;-Wno-format-security" APPEND)
endif()
