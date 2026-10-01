#include "SysTimer.hpp"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace esp32c6::low_level
{
std::uint32_t system_now_ms()
{
    return static_cast<std::uint32_t>(esp_timer_get_time() / 1000LL);
}
void system_sleep_ms(std::uint32_t duration_ms)
{
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}
}  // namespace esp32c6::low_level
