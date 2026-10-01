#pragma once

/* ====================================================================
 * @status: FROZEN
 * @note: Contract for External Interrupts (EXTI).
 *        Provides an asynchronous callback mechanism triggered by
 *        voltage level changes on a GPIO pin.
 * ==================================================================== */

#include <cstdint>
#include <uhal/Status.hpp>

namespace uhal
{

class IAdc
{
public:
    virtual ~IAdc() = default;

    virtual Status read_raw(std::uint16_t& sample) = 0;
};

}  // namespace uhal
