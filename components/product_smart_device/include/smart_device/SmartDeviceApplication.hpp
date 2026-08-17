#pragma once

#include <services/BinarySwitchService.hpp>
#include <uhal/Status.hpp>

namespace smart_device {

class ISwitchStateObserver {
public:
    virtual ~ISwitchStateObserver() = default;
    virtual void on_switch_state_changed(bool on) = 0;
};

class SmartDeviceApplication final {
public:
    explicit SmartDeviceApplication(services::BinarySwitchService& binary_switch,
                                    ISwitchStateObserver* observer = nullptr);

    uhal::Status initialize();
    uhal::Status on_short_press();
    uhal::Status set_switch(bool on);
    bool         is_switch_on() const;

private:
    void notify_if_changed(bool previous);

    services::BinarySwitchService& binary_switch_;
    ISwitchStateObserver*          observer_ = nullptr;
};

}  // namespace smart_device
