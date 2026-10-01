#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

class ByteWriter final
{
public:
    ByteWriter(std::uint8_t* data, std::size_t capacity) : data_{data}, capacity_{capacity} {}

    bool write_u8(std::uint8_t value)
    {
        return write_bytes(&value, 1U);
    }
    bool write_u16_le(std::uint16_t value)
    {
        const std::uint8_t bytes[2] = {static_cast<std::uint8_t>(value),
                                       static_cast<std::uint8_t>(value >> 8U)};
        return write_bytes(bytes, sizeof(bytes));
    }
    bool write_u32_le(std::uint32_t value)
    {
        const std::uint8_t bytes[4] = {
            static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(value >> 8U),
            static_cast<std::uint8_t>(value >> 16U), static_cast<std::uint8_t>(value >> 24U)};
        return write_bytes(bytes, sizeof(bytes));
    }
    bool write_bytes(const std::uint8_t* value, std::size_t size)
    {
        if (value == nullptr || position_ + size > capacity_)
        {
            overflow_ = true;
            return false;
        }
        for (std::size_t index = 0U; index < size; ++index) data_[position_++] = value[index];
        return true;
    }
    std::size_t size() const
    {
        return position_;
    }
    bool overflowed() const
    {
        return overflow_;
    }

private:
    std::uint8_t* data_;
    std::size_t   capacity_;
    std::size_t   position_ = 0U;
    bool          overflow_ = false;
};

}  // namespace libraries
