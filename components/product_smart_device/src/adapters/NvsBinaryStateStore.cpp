#include "NvsBinaryStateStore.hpp"

#include "nvs.h"

namespace smart_device {
namespace {

constexpr char         kNamespace[]   = "smartdev";
constexpr char         kSchemaKey[]   = "schema";
constexpr char         kRelayKey[]    = "relay_on";
constexpr std::uint8_t kSchemaVersion = 1U;

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
    nvs_handle_t handle{};
    if (nvs_open(kNamespace, NVS_READWRITE, &handle) != ESP_OK) {
        return uhal::Status::io_error;
    }
    esp_err_t error = nvs_set_u8(handle, kSchemaKey, kSchemaVersion);
    if (error == ESP_OK) {
        error = nvs_set_u8(handle, kRelayKey, state.on ? 1U : 0U);
    }
    if (error == ESP_OK) {
        error = nvs_commit(handle);
    }
    nvs_close(handle);
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

}  // namespace smart_device
