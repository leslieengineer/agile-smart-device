#include <smart_device/SwitchController.hpp>

namespace smart_device {

SwitchController::SwitchController(uhal::IGpio& relay, uhal::IGpio& led,
                                   IRelayStateRepository& repository)
    : relay_{relay}, led_{led}, repository_{repository} {}

uhal::Status SwitchController::restore() {
    RelayState restored{};
    if (repository_.load(restored) == uhal::Status::ok) {
        state_ = restored;
    } else {
        state_.on = false;
    }
    return apply();
}

uhal::Status SwitchController::toggle() {
    state_.on                       = !state_.on;
    const uhal::Status apply_status = apply();
    if (apply_status != uhal::Status::ok) {
        return apply_status;
    }
    return repository_.save(state_);
}

bool SwitchController::is_on() const {
    return state_.on;
}

uhal::Status SwitchController::apply() {
    const uhal::GpioLevel level        = state_.on ? uhal::GpioLevel::high : uhal::GpioLevel::low;
    const uhal::Status    relay_status = relay_.set(level);
    const uhal::Status    led_status   = led_.set(level);
    return relay_status == uhal::Status::ok && led_status == uhal::Status::ok
               ? uhal::Status::ok
               : uhal::Status::io_error;
}

}  // namespace smart_device
