#include <algorithm>
#include <array>
#include <protocols/ModbusRtuMaster.hpp>

namespace protocols
{

uhal::Status ModbusRtuMaster::send_request(std::uint8_t        slave_address,
                                           const std::uint8_t* protocol_data, std::size_t size)
{
    constexpr std::size_t maximum_pdu_size = 253;
    if (protocol_data == nullptr || size == 0 || size > maximum_pdu_size)
    {
        return uhal::Status::invalid_argument;
    }

    std::array<std::uint8_t, maximum_pdu_size + 1> frame{};
    frame[0] = slave_address;
    std::copy_n(protocol_data, size, frame.begin() + 1);
    return uart_.write(frame.data(), size + 1);
}

}  // namespace protocols
