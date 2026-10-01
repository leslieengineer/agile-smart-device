#include "Esp32C6Gpio.hpp"

#include "Gpio.hpp"
namespace esp32c6::adapters
{
namespace
{
bool physical_high(uhal::GpioLevel logical, bool active_low)
{
    const bool logical_high = logical == uhal::GpioLevel::high;
    return active_low ? !logical_high : logical_high;
}
low_level::GpioEdge to_edge(uhal::InterruptTrigger trigger)
{
    if (trigger == uhal::InterruptTrigger::rising_edge)
        return low_level::GpioEdge::rising;
    if (trigger == uhal::InterruptTrigger::falling_edge)
        return low_level::GpioEdge::falling;
    return low_level::GpioEdge::both;
}
}  // namespace
OutputPin::OutputPin(std::uint8_t pin, bool active_low, uhal::GpioLevel initial_level)
    : pin_{pin},
      active_low_{active_low},
      valid_{low_level::configure_output(pin, physical_high(initial_level, active_low))},
      level_{initial_level}
{
}
uhal::Status OutputPin::set(uhal::GpioLevel level)
{
    if (!valid_ || !low_level::gpio_write(pin_, physical_high(level, active_low_)))
        return uhal::Status::io_error;
    level_ = level;
    return uhal::Status::ok;
}
uhal::Status OutputPin::get(uhal::GpioLevel& level)
{
    if (!valid_)
        return uhal::Status::io_error;
    level = level_;
    return uhal::Status::ok;
}
InputPin::InputPin(std::uint8_t pin, bool pull_up, bool pull_down)
    : pin_{pin}, valid_{low_level::configure_input(pin, pull_up, pull_down)}
{
}
uhal::Status InputPin::set(uhal::GpioLevel)
{
    return uhal::Status::unsupported;
}
uhal::Status InputPin::get(uhal::GpioLevel& level)
{
    bool high = false;
    if (!valid_ || !low_level::gpio_read(pin_, high))
        return uhal::Status::io_error;
    level = high ? uhal::GpioLevel::high : uhal::GpioLevel::low;
    return uhal::Status::ok;
}
PinInterrupt::PinInterrupt(std::uint8_t pin) : pin_{pin} {}
PinInterrupt::~PinInterrupt()
{
    if (attached_)
        detach();
}
uhal::Status PinInterrupt::attach(uhal::InterruptTrigger trigger, Callback callback, void* context)
{
    if (attached_ || callback == nullptr || !low_level::install_gpio_isr_service() ||
        !low_level::set_gpio_edge(pin_, to_edge(trigger)) ||
        !low_level::add_gpio_isr_handler(pin_, callback, context) ||
        !low_level::disable_gpio_interrupt(pin_))
        return uhal::Status::io_error;
    attached_ = true;
    return uhal::Status::ok;
}
uhal::Status PinInterrupt::detach()
{
    if (!attached_)
        return uhal::Status::ok;
    if (!low_level::disable_gpio_interrupt(pin_) || !low_level::remove_gpio_isr_handler(pin_))
        return uhal::Status::io_error;
    attached_ = false;
    return uhal::Status::ok;
}
uhal::Status PinInterrupt::enable()
{
    return attached_ && low_level::enable_gpio_interrupt(pin_) ? uhal::Status::ok
                                                               : uhal::Status::io_error;
}
uhal::Status PinInterrupt::disable()
{
    return attached_ && low_level::disable_gpio_interrupt(pin_) ? uhal::Status::ok
                                                                : uhal::Status::io_error;
}
}  // namespace esp32c6::adapters
