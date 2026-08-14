#include <smart_device/SwitchController.hpp>
#include <uhal/fake/FakeGpio.hpp>

namespace {
class StubRepository final : public smart_device::IRelayStateRepository {
public:
    uhal::Status load(smart_device::RelayState& state) override {
        if (load_status == uhal::Status::ok) state = stored;
        return load_status;
    }
    uhal::Status save(const smart_device::RelayState& state) override {
        ++save_count;
        stored = state;
        return save_status;
    }
    smart_device::RelayState stored{};
    uhal::Status load_status = uhal::Status::ok;
    uhal::Status save_status = uhal::Status::ok;
    int save_count = 0;
};

bool level_is(uhal::fake::FakeGpio& gpio, uhal::GpioLevel expected) {
    uhal::GpioLevel actual{};
    return gpio.get(actual) == uhal::Status::ok && actual == expected;
}

bool restore_on() {
    uhal::fake::FakeGpio relay, led;
    StubRepository repo;
    repo.stored.on = true;
    smart_device::SwitchController controller{relay, led, repo};
    return controller.restore() == uhal::Status::ok && controller.is_on() &&
           level_is(relay, uhal::GpioLevel::high) && level_is(led, uhal::GpioLevel::high);
}

bool missing_defaults_off() {
    uhal::fake::FakeGpio relay, led;
    StubRepository repo;
    repo.load_status = uhal::Status::io_error;
    smart_device::SwitchController controller{relay, led, repo};
    return controller.restore() == uhal::Status::ok && !controller.is_on() &&
           level_is(relay, uhal::GpioLevel::low) && level_is(led, uhal::GpioLevel::low);
}

bool toggle_persists() {
    uhal::fake::FakeGpio relay, led;
    StubRepository repo;
    smart_device::SwitchController controller{relay, led, repo};
    controller.restore();
    return controller.toggle() == uhal::Status::ok && controller.is_on() &&
           repo.save_count == 1 && repo.stored.on && level_is(relay, uhal::GpioLevel::high);
}

bool save_failure_keeps_state() {
    uhal::fake::FakeGpio relay, led;
    StubRepository repo;
    repo.save_status = uhal::Status::io_error;
    smart_device::SwitchController controller{relay, led, repo};
    controller.restore();
    return controller.toggle() == uhal::Status::io_error && controller.is_on() &&
           level_is(relay, uhal::GpioLevel::high) && level_is(led, uhal::GpioLevel::high);
}
}  // namespace

int main() {
    return restore_on() && missing_defaults_off() && toggle_persists() &&
                   save_failure_keeps_state()
               ? 0
               : 1;
}
