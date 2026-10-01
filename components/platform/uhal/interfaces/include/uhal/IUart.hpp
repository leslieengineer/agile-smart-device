#pragma once

/* ====================================================================
 * @status: FROZEN
 * @note: Contract for External Interrupts (EXTI).
 *        Provides an asynchronous callback mechanism triggered by
 *        voltage level changes on a GPIO pin.
 * ==================================================================== */

#include <cstddef>
#include <cstdint>
#include <uhal/Status.hpp>

namespace uhal
{

class IUart
{
public:
    virtual ~IUart() = default;

    virtual Status write(const std::uint8_t* data, std::size_t size) = 0;
    virtual Status read(std::uint8_t* data, std::size_t capacity, std::size_t& received,
                        std::uint32_t timeout_ms)                    = 0;
};

}  // namespace uhal
