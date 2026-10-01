#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

class JsonScanner final
{
public:
    JsonScanner(const std::uint8_t* data, std::size_t size) : data_{data}, size_{size} {}

    bool get_bool(const char* key, bool& value) const
    {
        const std::size_t position = find_value(key);
        if (position >= size_)
            return false;
        if (matches(position, "true"))
        {
            value = true;
            return true;
        }
        if (matches(position, "false"))
        {
            value = false;
            return true;
        }
        return false;
    }

    bool get_u32(const char* key, std::uint32_t& value) const
    {
        std::size_t position = find_value(key);
        if (position >= size_ || data_[position] < '0' || data_[position] > '9')
            return false;
        std::uint32_t parsed = 0U;
        while (position < size_ && data_[position] >= '0' && data_[position] <= '9')
        {
            const std::uint32_t digit = data_[position++] - '0';
            if (parsed > (0xFFFFFFFFU - digit) / 10U)
                return false;
            parsed = parsed * 10U + digit;
        }
        value = parsed;
        return true;
    }

    bool get_string(const char* key, char* value, std::size_t capacity) const
    {
        std::size_t position = find_value(key);
        if (position >= size_ || data_[position++] != '"' || value == nullptr || capacity == 0U)
            return false;
        std::size_t output = 0U;
        while (position < size_ && data_[position] != '"')
        {
            if (output + 1U >= capacity)
                return false;
            if (data_[position] == '\\' && position + 1U < size_)
                ++position;
            value[output++] = static_cast<char>(data_[position++]);
        }
        if (position >= size_ || data_[position] != '"')
            return false;
        value[output] = '\0';
        return true;
    }

private:
    std::size_t find_value(const char* key) const
    {
        if (key == nullptr)
            return size_;
        for (std::size_t start = 0U; start < size_; ++start)
        {
            if (data_[start] != '"')
                continue;
            std::size_t input     = start + 1U;
            std::size_t key_index = 0U;
            while (input < size_ && key[key_index] != '\0' && data_[input] == key[key_index])
            {
                ++input;
                ++key_index;
            }
            if (key[key_index] != '\0' || input >= size_ || data_[input] != '"')
                continue;
            ++input;
            while (input < size_ && (data_[input] == ' ' || data_[input] == '\t')) ++input;
            if (input >= size_ || data_[input++] != ':')
                continue;
            while (input < size_ && (data_[input] == ' ' || data_[input] == '\t')) ++input;
            return input;
        }
        return size_;
    }
    bool matches(std::size_t position, const char* text) const
    {
        for (std::size_t index = 0U; text[index] != '\0'; ++index)
        {
            if (position + index >= size_ || data_[position + index] != text[index])
                return false;
        }
        return true;
    }

    const std::uint8_t* data_;
    std::size_t         size_;
};

}  // namespace libraries
