#pragma once
#include <uhal/IWatchdog.hpp>
namespace esp32c6::adapters
{
class Watchdog final : public uhal::IWatchdog
{
public:
    Watchdog();
    ~Watchdog() override;
    uhal::Status feed() override;
    bool         valid() const;

private:
    bool subscribed_ = false;
};
}  // namespace esp32c6::adapters
