#pragma once

#include <cstdint>
#include <uhal/IUart.hpp>

namespace protocols
{

class ModbusRtuMaster
{
public:
    explicit ModbusRtuMaster(uhal::IUart& uart) : uart_(uart) {}

    uhal::Status send_request(std::uint8_t slave_address, const std::uint8_t* protocol_data,
                              std::size_t size);

private:
    uhal::IUart& uart_;
};

}  // namespace protocols
