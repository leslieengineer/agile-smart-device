#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

inline std::uint32_t crc32(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t crc = 0xFFFFFFFFU;
    for (std::size_t index = 0U; index < size; ++index)
    {
        crc ^= data[index];
        for (std::uint8_t bit = 0U; bit < 8U; ++bit)
        {
            const std::uint32_t mask = 0U - (crc & 1U);
            crc                      = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

}  // namespace libraries
