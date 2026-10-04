cmake_minimum_required(VERSION 3.16)

# 1. Chọn profile đang được triển khai (local_switch | matter_node)
set(PRODUCT_PROFILE "matter_node" CACHE STRING "Firmware product profile")
set_property(CACHE PRODUCT_PROFILE PROPERTY STRINGS local_switch matter_node)
if(NOT PRODUCT_PROFILE STREQUAL "local_switch" AND NOT PRODUCT_PROFILE STREQUAL "matter_node")
    message(FATAL_ERROR
        "Unsupported PRODUCT_PROFILE='${PRODUCT_PROFILE}'. Supported values: local_switch, matter_node")
endif()

# 2. Báo ESP-IDF quét thêm thư mục app/ (components/ đã tự động được quét)
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/app")

# 3. Nếu build Matter thì nạp SDK esp-matter từ biến môi trường
if(PRODUCT_PROFILE STREQUAL "matter_node")
    if(NOT DEFINED ENV{ESP_MATTER_PATH})
        message(FATAL_ERROR "matter_node requires ESP_MATTER_PATH")
    endif()
    list(APPEND EXTRA_COMPONENT_DIRS
        "$ENV{ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/config/esp32/components"
        "$ENV{ESP_MATTER_PATH}/components"
    )
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(agile_smart_device)

# 4. Cờ biên dịch bắt buộc để SDK Matter (CHIP) không bị lỗi compiler
if(PRODUCT_PROFILE STREQUAL "matter_node")
    idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
    idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
    idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
endif()