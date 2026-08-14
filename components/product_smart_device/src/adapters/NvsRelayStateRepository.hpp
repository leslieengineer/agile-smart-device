#pragma once

#include <smart_device/IRelayStateRepository.hpp>

namespace smart_device {

class NvsRelayStateRepository final : public IRelayStateRepository {
public:
    uhal::Status load(RelayState& state) override;
    uhal::Status save(const RelayState& state) override;
};

}  // namespace smart_device
