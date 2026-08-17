#include "NvsBinaryStateStore.hpp"

#include "esp_log.h"
#include "nvs.h"

namespace smart_device {
namespace {

constexpr char          kTag[]         = "state-store";
constexpr char          kNamespace[]   = "smartdev";
constexpr char          kSchemaKey[]   = "schema";
constexpr char          kRelayKey[]    = "relay_on";
constexpr std::uint8_t  kSchemaVersion = 1U;
constexpr std::uint64_t kFlushDelayUs  = 500000U;

}  // namespace

uhal::Status NvsBinaryStateStore::load(services::BinaryState& state) {
    nvs_handle_t handle{};
    if (nvs_open(kNamespace, NVS_READONLY, &handle) != ESP_OK) {
        return uhal::Status::io_error;
    }
    std::uint8_t    schema       = 0U;
    std::uint8_t    relay        = 0U;
    const esp_err_t schema_error = nvs_get_u8(handle, kSchemaKey, &schema);
    const esp_err_t relay_error  = nvs_get_u8(handle, kRelayKey, &relay);
    nvs_close(handle);
    if (schema_error != ESP_OK || relay_error != ESP_OK || schema != kSchemaVersion) {
        return uhal::Status::io_error;
    }
    state.on = relay != 0U;
    return uhal::Status::ok;
}

uhal::Status NvsBinaryStateStore::save(const services::BinaryState& state) {
    const uhal::Status timer_status = initialize_timer();
    if (timer_status != uhal::Status::ok) return timer_status;

    pending_on_.store(state.on);
    const esp_err_t stop_error = esp_timer_stop(timer_);
    if (stop_error != ESP_OK && stop_error != ESP_ERR_INVALID_STATE) {
        return uhal::Status::io_error;
    }
    return esp_timer_start_once(timer_, kFlushDelayUs) == ESP_OK ? uhal::Status::ok
                                                                 : uhal::Status::io_error;
}

void NvsBinaryStateStore::flush_timer_callback(void* context) {
    auto* self = static_cast<NvsBinaryStateStore*>(context);
    if (self == nullptr) return;
    if (self->commit(self->pending_on_.load()) != uhal::Status::ok) {
        ESP_LOGE(kTag, "Failed to commit relay state");
    }
}

uhal::Status NvsBinaryStateStore::initialize_timer() {
    if (timer_ != nullptr) return uhal::Status::ok;

    esp_timer_create_args_t args{};
    args.callback = flush_timer_callback;
    args.arg = this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "relay_state";
    return esp_timer_create(&args, &timer_) == ESP_OK ? uhal::Status::ok
                                                       : uhal::Status::no_resources;
}

uhal::Status NvsBinaryStateStore::commit(bool on) {
    nvs_handle_t handle{};
    if (nvs_open(kNamespace, NVS_READWRITE, &handle) != ESP_OK) {
        return uhal::Status::io_error;
    }
    esp_err_t error = nvs_set_u8(handle, kSchemaKey, kSchemaVersion);
    if (error == ESP_OK) {
        error = nvs_set_u8(handle, kRelayKey, on ? 1U : 0U);
    }
    if (error == ESP_OK) {
        error = nvs_commit(handle);
    }
    nvs_close(handle);
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

}  // namespace smart_device
