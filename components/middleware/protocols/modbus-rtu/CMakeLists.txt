add_library(modbus_rtu STATIC src/ModbusRtuMaster.cpp)
target_include_directories(modbus_rtu PUBLIC include)
target_link_libraries(modbus_rtu PUBLIC uhal_interfaces)
