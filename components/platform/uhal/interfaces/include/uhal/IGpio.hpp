#pragma once

/* ====================================================================
 * @status: FROZEN
 * @note: Contract for External Interrupts (EXTI).
 *        Provides an asynchronous callback mechanism triggered by
 *        voltage level changes on a GPIO pin.
 * ==================================================================== */

#include <uhal/Status.hpp>

namespace uhal
{

enum class GpioLevel : unsigned char
{
    low,
    high,
};

class IGpio
{
public:
    virtual ~IGpio() = default;

    virtual Status set(GpioLevel level)  = 0;
    virtual Status get(GpioLevel& level) = 0;
};

}  // namespace uhal
