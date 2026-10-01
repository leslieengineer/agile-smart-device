#pragma once

#include <cstdint>

namespace uhal
{

enum class Status : std::uint8_t
{
    ok,
    timeout,
    nack,
    bus_error,
    busy,
    io_error,
    unsupported,
    invalid_argument,
    not_found,
    no_resources,
    corrupt,
    not_ready,
    denied,
    aborted,
};

}  // namespace uhal
