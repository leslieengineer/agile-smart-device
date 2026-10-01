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

enum class InterruptTrigger : unsigned char
{
    rising_edge,
    falling_edge,
    both_edges
};

class IGpioInterrupt
{
public:
    using Callback = void (*)(void* context);

    virtual ~IGpioInterrupt() = default;

    /**
     * @brief Attach a callback function to a specific hardware trigger event.
     *
     * @param trigger  The edge transition that will fire the interrupt.
     * @param callback The function to execute within the ISR context.
     * @param context  The instance pointer passed back to the callback.
     * @return Status  Ok on success, or an appropriate error code.
     */
    virtual Status attach(InterruptTrigger trigger, Callback callback, void* context) = 0;

    /**
     * @brief Detach the callback and release the interrupt hardware resource.
     */
    virtual Status detach() = 0;

    /**
     * @brief Temporarily unmask/enable the interrupt without reattaching.
     */
    virtual Status enable() = 0;

    /**
     * @brief Temporarily mask/disable the interrupt. Useful for software debouncing.
     */
    virtual Status disable() = 0;
};

}  // namespace uhal