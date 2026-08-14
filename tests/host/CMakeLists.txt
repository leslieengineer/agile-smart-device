cmake_minimum_required(VERSION 3.20)
project(smart_device_host_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(smart_device_application_tests
    BinarySwitchCompositionTests.cpp
    ../../components/product_smart_device/src/SmartDeviceApplication.cpp
    ../../external/agile-firmware-framework/components/services/binary_switch/src/BinarySwitchService.cpp
)
target_include_directories(smart_device_application_tests PRIVATE
    ../../components/product_smart_device/include
    ../../external/agile-firmware-framework/components/services/binary_switch/include
    ../../external/agile-firmware-framework/components/uhal/core/include
    ../../external/agile-firmware-framework/components/uhal/interfaces/include
    ../../external/agile-firmware-framework/components/platform/fake/include
)

enable_testing()
add_test(NAME smart_device_application_tests COMMAND smart_device_application_tests)
foreach(CHECK_NAME boundaries dynamic catalog)
    add_test(
        NAME layer_${CHECK_NAME}_check
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/../../tools/check_layer_boundaries.py
                --root ${CMAKE_CURRENT_LIST_DIR}/../.. --check ${CHECK_NAME}
    )
endforeach()
