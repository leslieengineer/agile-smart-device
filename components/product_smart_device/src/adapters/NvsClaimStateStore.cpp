#include "NvsClaimStateStore.hpp"

#include "esp_log.h"
#include "nvs.h"

namespace smart_device {
namespace {

constexpr char kTag[] = "claim-state";
constexpr char kNamespace[] = "rhophi_state";
constexpr char kAttemptsKey[] = "attempts";
constexpr char kLockoutLevelKey[] = "lock_level";
constexpr char kLockoutMsKey[] = "lock_ms";
constexpr char kClaimedKey[] = "claimed";
constexpr UBaseType_t kWorkerPriority = 3U;
constexpr std::uint32_t kWorkerStackSize = 3072U;

uhal::Status status_from_nvs(esp_err_t error) {
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

}  // namespace

uhal::Status NvsClaimStateStore::load(ClaimPersistentState& state) {
    state = {};
    nvs_handle_t handle = 0U;
    const esp_err_t open_error = nvs_open(kNamespace, NVS_READONLY, &handle);
    if (open_error == ESP_ERR_NVS_NOT_FOUND) return uhal::Status::ok;
    if (open_error != ESP_OK) return uhal::Status::io_error;

    std::uint8_t claimed = 0U;
    esp_err_t error = nvs_get_u8(handle, kAttemptsKey, &state.attempts);
    if (error != ESP_OK && error != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return uhal::Status::io_error;
    }
    error = nvs_get_u8(handle, kLockoutLevelKey, &state.lockout_level);
    if (error != ESP_OK && error != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return uhal::Status::io_error;
    }
    error = nvs_get_u32(handle, kLockoutMsKey, &state.lockout_ms);
    if (error != ESP_OK && error != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return uhal::Status::io_error;
    }
    error = nvs_get_u8(handle, kClaimedKey, &claimed);
    nvs_close(handle);
    if (error != ESP_OK && error != ESP_ERR_NVS_NOT_FOUND) return uhal::Status::io_error;
    state.claimed = claimed != 0U;
    return uhal::Status::ok;
}

uhal::Status NvsClaimStateStore::save(const ClaimPersistentState& state) {
    const uhal::Status worker_status = ensure_worker();
    if (worker_status != uhal::Status::ok) return worker_status;
    const WriteRequest request{state};
    return xQueueOverwrite(queue_, &request) == pdPASS ? uhal::Status::ok : uhal::Status::busy;
}

uhal::Status NvsClaimStateStore::clear() {
    if (ensure_io_mutex() != uhal::Status::ok) return uhal::Status::no_resources;
    if (xSemaphoreTake(io_mutex_, portMAX_DELAY) != pdTRUE) return uhal::Status::busy;
    if (queue_ != nullptr) (void)xQueueReset(queue_);
    nvs_handle_t handle = 0U;
    const esp_err_t open_error = nvs_open(kNamespace, NVS_READWRITE, &handle);
    if (open_error != ESP_OK) {
        xSemaphoreGive(io_mutex_);
        return status_from_nvs(open_error);
    }
    esp_err_t error = nvs_erase_all(handle);
    if (error == ESP_OK) error = nvs_commit(handle);
    nvs_close(handle);
    xSemaphoreGive(io_mutex_);
    return status_from_nvs(error);
}

void NvsClaimStateStore::worker_entry(void* context) {
    auto* self = static_cast<NvsClaimStateStore*>(context);
    if (self != nullptr) self->worker();
    vTaskDelete(nullptr);
}

void NvsClaimStateStore::worker() {
    WriteRequest request{};
    while (xQueueReceive(queue_, &request, portMAX_DELAY) == pdTRUE) {
        if (xSemaphoreTake(io_mutex_, portMAX_DELAY) != pdTRUE) continue;
        const uhal::Status status = write_now(request.state);
        xSemaphoreGive(io_mutex_);
        if (status != uhal::Status::ok) ESP_LOGE(kTag, "Failed to persist claim state");
    }
}

uhal::Status NvsClaimStateStore::ensure_worker() {
    if (queue_ != nullptr && worker_task_ != nullptr) return uhal::Status::ok;
    if (ensure_io_mutex() != uhal::Status::ok) return uhal::Status::no_resources;
    queue_ = xQueueCreate(1U, sizeof(WriteRequest));
    if (queue_ == nullptr) return uhal::Status::no_resources;
    if (xTaskCreate(worker_entry, "claim-state", kWorkerStackSize, this, kWorkerPriority,
                    &worker_task_) != pdPASS) {
        vQueueDelete(queue_);
        queue_ = nullptr;
        return uhal::Status::no_resources;
    }
    return uhal::Status::ok;
}

uhal::Status NvsClaimStateStore::ensure_io_mutex() {
    if (io_mutex_ != nullptr) return uhal::Status::ok;
    io_mutex_ = xSemaphoreCreateMutex();
    return io_mutex_ == nullptr ? uhal::Status::no_resources : uhal::Status::ok;
}

uhal::Status NvsClaimStateStore::write_now(const ClaimPersistentState& state) {
    nvs_handle_t handle = 0U;
    const esp_err_t open_error = nvs_open(kNamespace, NVS_READWRITE, &handle);
    if (open_error != ESP_OK) return status_from_nvs(open_error);
    esp_err_t error = nvs_set_u8(handle, kAttemptsKey, state.attempts);
    if (error == ESP_OK) error = nvs_set_u8(handle, kLockoutLevelKey, state.lockout_level);
    if (error == ESP_OK) error = nvs_set_u32(handle, kLockoutMsKey, state.lockout_ms);
    if (error == ESP_OK) error = nvs_set_u8(handle, kClaimedKey, state.claimed ? 1U : 0U);
    if (error == ESP_OK) error = nvs_commit(handle);
    nvs_close(handle);
    return status_from_nvs(error);
}

}  // namespace smart_device
