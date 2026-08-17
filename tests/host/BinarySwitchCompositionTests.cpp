#include <smart_device/SmartDeviceApplication.hpp>
#include <uhal/fake/FakeGpio.hpp>

namespace {

class StubStore final : public services::IBinaryStateStore {
public:
    uhal::Status load(services::BinaryState& state) override {
        if (load_status == uhal::Status::ok) state = stored;
        return load_status;
    }

    uhal::Status save(const services::BinaryState& state) override {
        stored = state;
        ++save_count;
        return save_status;
    }

    services::BinaryState stored{};
    uhal::Status          load_status = uhal::Status::ok;
    uhal::Status          save_status = uhal::Status::ok;
    int                   save_count  = 0;
};

class StateObserver final : public smart_device::ISwitchStateObserver {
public:
    void on_switch_state_changed(bool on) override {
        last_state = on;
        ++notification_count;
    }

    bool last_state         = false;
    int  notification_count = 0;
};

bool initialize_restores_on() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    store.stored.on = true;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.initialize() == uhal::Status::ok && application.is_switch_on();
}

bool initialize_missing_state_defaults_off() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    store.load_status = uhal::Status::io_error;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.initialize() == uhal::Status::ok && !application.is_switch_on();
}

bool short_press_maps_to_one_toggle() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.initialize() == uhal::Status::ok &&
           application.on_short_press() == uhal::Status::ok && application.is_switch_on() &&
           store.save_count == 1 && store.stored.on;
}

bool explicit_set_commands_are_forwarded() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.set_switch(true) == uhal::Status::ok && application.is_switch_on() &&
           application.set_switch(false) == uhal::Status::ok && !application.is_switch_on() &&
           store.save_count == 2;
}

bool duplicate_commands_do_not_rewrite_state() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.set_switch(true) == uhal::Status::ok &&
           application.set_switch(true) == uhal::Status::ok && store.save_count == 1;
}

bool observers_receive_committed_changes() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    StateObserver        observer;
    store.stored.on = true;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service, &observer};

    if (application.initialize() != uhal::Status::ok || observer.notification_count != 1 ||
        !observer.last_state) {
        return false;
    }
    if (application.set_switch(false) != uhal::Status::ok || observer.notification_count != 2 ||
        observer.last_state) {
        return false;
    }
    return application.set_switch(false) == uhal::Status::ok && observer.notification_count == 2;
}

bool service_errors_are_propagated() {
    uhal::fake::FakeGpio relay, indicator;
    StubStore            store;
    store.save_status = uhal::Status::io_error;
    services::BinarySwitchService service{relay, indicator, store};
    smart_device::SmartDeviceApplication application{service};

    return application.on_short_press() == uhal::Status::io_error && application.is_switch_on();
}

}  // namespace

int main() {
    return initialize_restores_on() && initialize_missing_state_defaults_off() &&
                   short_press_maps_to_one_toggle() && explicit_set_commands_are_forwarded() &&
                   duplicate_commands_do_not_rewrite_state() && observers_receive_committed_changes() &&
                   service_errors_are_propagated()
               ? 0
               : 1;
}
