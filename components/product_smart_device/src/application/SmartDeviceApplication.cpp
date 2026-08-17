#include <smart_device/SmartDeviceApplication.hpp>

namespace smart_device {

SmartDeviceApplication::SmartDeviceApplication(services::BinarySwitchService& binary_switch,
                                               ISwitchStateObserver* observer)
    : binary_switch_{binary_switch}, observer_{observer} {}

uhal::Status SmartDeviceApplication::initialize() {
    const uhal::Status status = binary_switch_.restore();
    if (status == uhal::Status::ok && observer_ != nullptr) {
        observer_->on_switch_state_changed(binary_switch_.is_on());
    }
    return status;
}

uhal::Status SmartDeviceApplication::on_short_press() {
    const bool         previous = binary_switch_.is_on();
    const uhal::Status status   = binary_switch_.toggle();
    notify_if_changed(previous);
    return status;
}

uhal::Status SmartDeviceApplication::set_switch(bool on) {
    const bool previous = binary_switch_.is_on();
    if (previous == on) return uhal::Status::ok;

    const uhal::Status status = binary_switch_.set(on);
    notify_if_changed(previous);
    return status;
}

bool SmartDeviceApplication::is_switch_on() const {
    return binary_switch_.is_on();
}

void SmartDeviceApplication::notify_if_changed(bool previous) {
    const bool current = binary_switch_.is_on();
    if (observer_ != nullptr && current != previous) {
        observer_->on_switch_state_changed(current);
    }
}

}  // namespace smart_device
