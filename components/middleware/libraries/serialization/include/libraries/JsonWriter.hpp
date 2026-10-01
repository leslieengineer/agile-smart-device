#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

class JsonWriter final
{
public:
    JsonWriter(std::uint8_t* data, std::size_t capacity) : data_{data}, capacity_{capacity} {}

    bool begin_object()
    {
        first_ = true;
        return put('{');
    }
    bool end_object()
    {
        return put('}') && terminate();
    }
    bool field_bool(const char* key, bool value)
    {
        return field_prefix(key) && append(value ? "true" : "false");
    }
    bool field_u32(const char* key, std::uint32_t value)
    {
        if (!field_prefix(key))
            return false;
        char        digits[10]{};
        std::size_t count = 0U;
        do
        {
            digits[count++] = static_cast<char>('0' + (value % 10U));
            value /= 10U;
        } while (value != 0U && count < sizeof(digits));
        while (count > 0U)
        {
            if (!put(digits[--count]))
                return false;
        }
        return true;
    }
    bool field_string(const char* key, const char* value)
    {
        if (!field_prefix(key) || !put('"'))
            return false;
        for (std::size_t index = 0U; value != nullptr && value[index] != '\0'; ++index)
        {
            const char character = value[index];
            if ((character == '"' || character == '\\') && !put('\\'))
                return false;
            if (!put(character))
                return false;
        }
        return put('"');
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
    bool field_prefix(const char* key)
    {
        if (!first_ && !put(','))
            return false;
        first_ = false;
        return put('"') && append(key) && put('"') && put(':');
    }
    bool append(const char* text)
    {
        if (text == nullptr)
            return false;
        for (std::size_t index = 0U; text[index] != '\0'; ++index)
        {
            if (!put(text[index]))
                return false;
        }
        return true;
    }
    bool put(char value)
    {
        if (position_ >= capacity_)
        {
            overflow_ = true;
            return false;
        }
        data_[position_++] = static_cast<std::uint8_t>(value);
        return true;
    }
    bool terminate()
    {
        if (position_ >= capacity_)
        {
            overflow_ = true;
            return false;
        }
        data_[position_] = 0U;
        return true;
    }

    std::uint8_t* data_;
    std::size_t   capacity_;
    std::size_t   position_ = 0U;
    bool          first_    = true;
    bool          overflow_ = false;
};

}  // namespace libraries
