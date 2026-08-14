cmake_minimum_required(VERSION 3.20)
project(smart_device_host_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(switch_controller_tests
    SwitchControllerTests.cpp
    ../../components/product_smart_device/src/SwitchController.cpp
)
target_include_directories(switch_controller_tests PRIVATE
    ../../components/product_smart_device/include
    ../../external/agile-firmware-framework/components/uhal/core/include
    ../../external/agile-firmware-framework/components/uhal/interfaces/include
    ../../external/agile-firmware-framework/components/platform/fake/include
)
enable_testing()
add_test(NAME switch_controller_tests COMMAND switch_controller_tests)
