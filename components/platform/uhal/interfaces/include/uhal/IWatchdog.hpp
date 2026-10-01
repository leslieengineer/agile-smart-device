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

class IWatchdog
{
public:
    virtual ~IWatchdog() = default;

    virtual Status feed() = 0;
};

}  // namespace uhal
