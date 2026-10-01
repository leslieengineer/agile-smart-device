#include "Watchdog.hpp"

#include "esp_task_wdt.h"
namespace esp32c6::low_level
{
bool watchdog_subscribe_current_task()
{
    return esp_task_wdt_add(nullptr) == ESP_OK;
}
bool watchdog_unsubscribe_current_task()
{
    return esp_task_wdt_delete(nullptr) == ESP_OK;
}
bool watchdog_feed()
{
    return esp_task_wdt_reset() == ESP_OK;
}
}  // namespace esp32c6::low_level
