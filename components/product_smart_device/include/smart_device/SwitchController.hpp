#pragma once

#include <smart_device/IRelayStateRepository.hpp>
#include <uhal/IGpio.hpp>

namespace smart_device {

class SwitchController final {
public:
    SwitchController(uhal::IGpio& relay, uhal::IGpio& led, IRelayStateRepository& repository);

    uhal::Status restore();
    uhal::Status toggle();
    bool         is_on() const;

private:
    uhal::Status apply();

    uhal::IGpio&           relay_;
    uhal::IGpio&           led_;
    IRelayStateRepository& repository_;
    RelayState             state_{};
};

}  // namespace smart_device
