#include "Gpio.hpp"

#include "driver/gpio.h"
#include "esp_err.h"
namespace esp32c6::low_level
{
namespace
{
gpio_num_t to_gpio(std::uint8_t pin)
{
    return static_cast<gpio_num_t>(pin);
}
bool valid_pin(std::uint8_t pin)
{
    return GPIO_IS_VALID_GPIO(to_gpio(pin));
}
}  // namespace
bool configure_output(std::uint8_t pin, bool initial_high)
{
    if (!valid_pin(pin))
        return false;
    gpio_config_t config{};
    config.pin_bit_mask = 1ULL << pin;
    config.mode         = GPIO_MODE_OUTPUT;
    config.pull_up_en   = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type    = GPIO_INTR_DISABLE;
    return gpio_config(&config) == ESP_OK &&
           gpio_set_level(to_gpio(pin), initial_high ? 1U : 0U) == ESP_OK;
}
bool configure_input(std::uint8_t pin, bool pull_up, bool pull_down)
{
    if (!valid_pin(pin))
        return false;
    gpio_config_t config{};
    config.pin_bit_mask = 1ULL << pin;
    config.mode         = GPIO_MODE_INPUT;
    config.pull_up_en   = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    config.pull_down_en = pull_down ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    config.intr_type    = GPIO_INTR_DISABLE;
    return gpio_config(&config) == ESP_OK;
}
bool gpio_write(std::uint8_t pin, bool high)
{
    return valid_pin(pin) && gpio_set_level(to_gpio(pin), high ? 1U : 0U) == ESP_OK;
}
bool gpio_read(std::uint8_t pin, bool& high)
{
    if (!valid_pin(pin))
        return false;
    high = gpio_get_level(to_gpio(pin)) != 0;
    return true;
}
bool install_gpio_isr_service()
{
    const esp_err_t error = gpio_install_isr_service(0);
    return error == ESP_OK || error == ESP_ERR_INVALID_STATE;
}
bool set_gpio_edge(std::uint8_t pin, GpioEdge edge)
{
    gpio_int_type_t type = GPIO_INTR_ANYEDGE;
    if (edge == GpioEdge::rising)
        type = GPIO_INTR_POSEDGE;
    else if (edge == GpioEdge::falling)
        type = GPIO_INTR_NEGEDGE;
    return valid_pin(pin) && gpio_set_intr_type(to_gpio(pin), type) == ESP_OK;
}
bool add_gpio_isr_handler(std::uint8_t pin, GpioIsrHandler handler, void* context)
{
    return valid_pin(pin) && handler != nullptr &&
           gpio_isr_handler_add(to_gpio(pin), handler, context) == ESP_OK;
}
bool remove_gpio_isr_handler(std::uint8_t pin)
{
    return valid_pin(pin) && gpio_isr_handler_remove(to_gpio(pin)) == ESP_OK;
}
bool enable_gpio_interrupt(std::uint8_t pin)
{
    return valid_pin(pin) && gpio_intr_enable(to_gpio(pin)) == ESP_OK;
}
bool disable_gpio_interrupt(std::uint8_t pin)
{
    return valid_pin(pin) && gpio_intr_disable(to_gpio(pin)) == ESP_OK;
}
}  // namespace esp32c6::low_level
