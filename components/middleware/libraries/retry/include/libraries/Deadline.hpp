#pragma once

#include <cstdint>

namespace libraries
{

class Deadline final
{
public:
    Deadline(std::uint32_t start_ms, std::uint32_t timeout_ms)
        : start_ms_{start_ms}, timeout_ms_{timeout_ms}
    {
    }

    bool expired(std::uint32_t now_ms) const
    {
        return (now_ms - start_ms_) >= timeout_ms_;
    }
    std::uint32_t elapsed_ms(std::uint32_t now_ms) const
    {
        return now_ms - start_ms_;
    }

private:
    std::uint32_t start_ms_;
    std::uint32_t timeout_ms_;
};

}  // namespace libraries
