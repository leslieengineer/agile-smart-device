#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

class ByteReader final
{
public:
    ByteReader(const std::uint8_t* data, std::size_t size) : data_{data}, size_{size} {}

    bool read_u8(std::uint8_t& value)
    {
        return read_bytes(&value, 1U);
    }
    bool read_u16_le(std::uint16_t& value)
    {
        std::uint8_t bytes[2]{};
        if (!read_bytes(bytes, sizeof(bytes)))
            return false;
        value = static_cast<std::uint16_t>(bytes[0]) |
                static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[1]) << 8U);
        return true;
    }
    bool read_u32_le(std::uint32_t& value)
    {
        std::uint8_t bytes[4]{};
        if (!read_bytes(bytes, sizeof(bytes)))
            return false;
        value = static_cast<std::uint32_t>(bytes[0]) |
                (static_cast<std::uint32_t>(bytes[1]) << 8U) |
                (static_cast<std::uint32_t>(bytes[2]) << 16U) |
                (static_cast<std::uint32_t>(bytes[3]) << 24U);
        return true;
    }
    bool read_bytes(std::uint8_t* value, std::size_t size)
    {
        if (value == nullptr || position_ + size > size_)
            return false;
        for (std::size_t index = 0U; index < size; ++index) value[index] = data_[position_++];
        return true;
    }
    std::size_t remaining() const
    {
        return size_ - position_;
    }

private:
    const std::uint8_t* data_;
    std::size_t         size_;
    std::size_t         position_ = 0U;
};

}  // namespace libraries
