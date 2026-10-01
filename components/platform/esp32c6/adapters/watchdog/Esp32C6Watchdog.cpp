#include "Esp32C6Watchdog.hpp"

#include "Watchdog.hpp"
namespace esp32c6::adapters
{
Watchdog::Watchdog() : subscribed_{low_level::watchdog_subscribe_current_task()} {}
Watchdog::~Watchdog()
{
    if (subscribed_)
        low_level::watchdog_unsubscribe_current_task();
}
uhal::Status Watchdog::feed()
{
    return subscribed_ && low_level::watchdog_feed() ? uhal::Status::ok : uhal::Status::io_error;
}
bool Watchdog::valid() const
{
    return subscribed_;
}
}  // namespace esp32c6::adapters
