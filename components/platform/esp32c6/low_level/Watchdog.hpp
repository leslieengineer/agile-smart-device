#pragma once
namespace esp32c6::low_level
{
bool watchdog_subscribe_current_task();
bool watchdog_unsubscribe_current_task();
bool watchdog_feed();
}  // namespace esp32c6::low_level
