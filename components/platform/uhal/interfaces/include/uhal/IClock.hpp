#pragma once

/* ====================================================================
 * @status: FROZEN
 * @note: Contract for External Interrupts (EXTI).
 *        Provides an asynchronous callback mechanism triggered by
 *        voltage level changes on a GPIO pin.
 * ==================================================================== */

#include <cstdint>

namespace uhal
{

class IClock
{
public:
    virtual ~IClock() = default;

    virtual std::uint32_t now_ms() const                      = 0;
    virtual void          sleep_ms(std::uint32_t duration_ms) = 0;
};

}  // namespace uhal
