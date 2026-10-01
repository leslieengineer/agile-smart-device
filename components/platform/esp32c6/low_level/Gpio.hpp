#pragma once
#include <cstdint>
namespace esp32c6::low_level
{
enum class GpioEdge : std::uint8_t
{
    rising,
    falling,
    both
};
using GpioIsrHandler = void (*)(void* context);
bool configure_output(std::uint8_t pin, bool initial_high);
bool configure_input(std::uint8_t pin, bool pull_up, bool pull_down);
bool gpio_write(std::uint8_t pin, bool high);
bool gpio_read(std::uint8_t pin, bool& high);
bool install_gpio_isr_service();
bool set_gpio_edge(std::uint8_t pin, GpioEdge edge);
bool add_gpio_isr_handler(std::uint8_t pin, GpioIsrHandler handler, void* context);
bool remove_gpio_isr_handler(std::uint8_t pin);
bool enable_gpio_interrupt(std::uint8_t pin);
bool disable_gpio_interrupt(std::uint8_t pin);
}  // namespace esp32c6::low_level
