#pragma once

#include <uhal/Status.hpp>

namespace smart_device {

struct RelayState {
    bool on = false;
};

class IRelayStateRepository {
public:
    virtual ~IRelayStateRepository()                   = default;
    virtual uhal::Status load(RelayState& state)       = 0;
    virtual uhal::Status save(const RelayState& state) = 0;
};

}  // namespace smart_device
