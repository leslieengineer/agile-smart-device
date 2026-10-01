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

class II2c
{
public:
    virtual ~II2c() = default;

    virtual Status write(std::uint8_t address, const std::uint8_t* data, std::size_t size) = 0;
    virtual Status read(std::uint8_t address, std::uint8_t* data, std::size_t size)        = 0;
};

}  // namespace uhal
