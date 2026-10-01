#pragma once

#include <cstddef>
#include <cstdint>
#include <uhal/Status.hpp>

namespace services
{

class IConfigStore
{
public:
    virtual ~IConfigStore()                                                   = default;
    virtual uhal::Status get_blob(const char* key, std::uint8_t* value, std::size_t capacity,
                                  std::size_t& value_size) const              = 0;
    virtual uhal::Status set_blob(const char* key, const std::uint8_t* value,
                                  std::size_t value_size)                     = 0;
    virtual uhal::Status get_u32(const char* key, std::uint32_t& value) const = 0;
    virtual uhal::Status set_u32(const char* key, std::uint32_t value)        = 0;
    virtual uhal::Status get_bool(const char* key, bool& value) const         = 0;
    virtual uhal::Status set_bool(const char* key, bool value)                = 0;
    virtual uhal::Status erase(const char* key)                               = 0;
    virtual uhal::Status commit()                                             = 0;
};

}  // namespace services
