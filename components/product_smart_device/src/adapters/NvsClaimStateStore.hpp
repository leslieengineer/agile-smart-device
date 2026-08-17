#pragma once

#include "../matter/RhophiClaimProtocol.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

namespace smart_device {

class NvsClaimStateStore final : public IClaimStateStore {
public:
    uhal::Status load(ClaimPersistentState& state) override;
    uhal::Status save(const ClaimPersistentState& state) override;
    uhal::Status clear() override;

private:
    struct WriteRequest {
        ClaimPersistentState state{};
    };

    static void worker_entry(void* context);
    void worker();
    uhal::Status ensure_worker();
    uhal::Status ensure_io_mutex();
    static uhal::Status write_now(const ClaimPersistentState& state);

    QueueHandle_t queue_ = nullptr;
    TaskHandle_t worker_task_ = nullptr;
    SemaphoreHandle_t io_mutex_ = nullptr;
};

}  // namespace smart_device
