cmake_minimum_required(VERSION 3.16)

set(PRODUCT_PROFILE "local_switch" CACHE STRING "Firmware product profile")
set_property(CACHE PRODUCT_PROFILE PROPERTY STRINGS local_switch gateway_node)

list(APPEND EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/imports/common/components"
    "${CMAKE_CURRENT_LIST_DIR}/imports/local_switch/components"
)

if(PRODUCT_PROFILE STREQUAL "gateway_node")
    list(APPEND EXTRA_COMPONENT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/imports/gateway_node/components"
    )
elseif(NOT PRODUCT_PROFILE STREQUAL "local_switch")
    message(FATAL_ERROR "Unsupported PRODUCT_PROFILE: ${PRODUCT_PROFILE}")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(agile_smart_device)
