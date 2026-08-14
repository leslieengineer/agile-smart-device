#include <smart_device/SmartDeviceApplication.hpp>

namespace smart_device {

SmartDeviceApplication::SmartDeviceApplication(services::BinarySwitchService& binary_switch)
    : binary_switch_{binary_switch} {}

uhal::Status SmartDeviceApplication::initialize() {
    return binary_switch_.restore();
}

uhal::Status SmartDeviceApplication::on_short_press() {
    return binary_switch_.toggle();
}

uhal::Status SmartDeviceApplication::set_switch(bool on) {
    return binary_switch_.set(on);
}

bool SmartDeviceApplication::is_switch_on() const {
    return binary_switch_.is_on();
}

}  // namespace smart_device
