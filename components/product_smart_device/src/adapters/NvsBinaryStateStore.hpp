#pragma once

#include <atomic>

#include <services/IBinaryStateStore.hpp>

#include "esp_timer.h"

namespace smart_device {

class NvsBinaryStateStore final : public services::IBinaryStateStore {
public:
    uhal::Status load(services::BinaryState& state) override;
    uhal::Status save(const services::BinaryState& state) override;

private:
    static void flush_timer_callback(void* context);
    uhal::Status initialize_timer();
    uhal::Status commit(bool on);

    esp_timer_handle_t timer_ = nullptr;
    std::atomic_bool   pending_on_{false};
};

}  // namespace smart_device
