#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

inline std::uint16_t crc16_modbus(const std::uint8_t* data, std::size_t size)
{
    std::uint16_t crc = 0xFFFFU;

    for (std::size_t index = 0; index < size; ++index)
    {
        crc ^= data[index];
        for (std::uint8_t bit = 0; bit < 8U; ++bit)
        {
            crc = (crc & 1U) != 0U ? static_cast<std::uint16_t>((crc >> 1U) ^ 0xA001U)
                                   : static_cast<std::uint16_t>(crc >> 1U);
        }
    }
    return crc;
}

}  // namespace libraries
