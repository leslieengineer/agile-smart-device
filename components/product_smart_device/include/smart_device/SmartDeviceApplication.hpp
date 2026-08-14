#pragma once

#include <services/BinarySwitchService.hpp>
#include <uhal/Status.hpp>

namespace smart_device {

class SmartDeviceApplication final {
public:
    explicit SmartDeviceApplication(services::BinarySwitchService& binary_switch);

    uhal::Status initialize();
    uhal::Status on_short_press();
    uhal::Status set_switch(bool on);
    bool         is_switch_on() const;

private:
    services::BinarySwitchService& binary_switch_;
};

}  // namespace smart_device
