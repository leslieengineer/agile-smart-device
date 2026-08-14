#pragma once

#include <services/IBinaryStateStore.hpp>

namespace smart_device {

class NvsBinaryStateStore final : public services::IBinaryStateStore {
public:
    uhal::Status load(services::BinaryState& state) override;
    uhal::Status save(const services::BinaryState& state) override;
};

}  // namespace smart_device
