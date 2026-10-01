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

struct CanFrame
{
    std::uint32_t identifier;
    std::uint8_t  data[64];
    std::size_t   size;
    bool          extended_identifier;
};

class ICan
{
public:
    virtual ~ICan() = default;

    virtual Status send(const CanFrame& frame)                        = 0;
    virtual Status receive(CanFrame& frame, std::uint32_t timeout_ms) = 0;
};

}  // namespace uhal
