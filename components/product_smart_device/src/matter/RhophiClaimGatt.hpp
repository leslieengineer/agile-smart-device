#pragma once

#include <array>
#include <cstdint>

#include "RhophiClaimProtocol.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct ble_gatt_access_ctxt;

namespace smart_device {

class IClaimGattActions {
public:
    virtual ~IClaimGattActions() = default;
    virtual void identify_claim_device() = 0;
};

class RhophiClaimGatt final {
public:
    enum class Characteristic : std::uint8_t { identity, challenge, response, state, identify, cancel };

    RhophiClaimGatt(RhophiClaimProtocol& protocol, IClaimGattActions& actions);
    uhal::Status initialize();
    uhal::Status open_window(std::uint32_t now_ms, std::uint32_t duration_ms);
    void cancel();
    void release_session();
    uhal::Status mark_commissioned();
    uhal::Status factory_reset();

private:
    static constexpr std::uint16_t kNoConnection = 0xFFFFU;

    static int access_callback(std::uint16_t connection_handle, std::uint16_t attribute_handle,
                               ble_gatt_access_ctxt* context, void* argument);
    int handle_access(Characteristic characteristic, std::uint16_t connection_handle,
                      ble_gatt_access_ctxt* context);
    int read_identity(ble_gatt_access_ctxt* context);
    int write_challenge(std::uint16_t connection_handle, ble_gatt_access_ctxt* context);
    int read_response(std::uint16_t connection_handle, ble_gatt_access_ctxt* context);
    int read_state(ble_gatt_access_ctxt* context);
    int write_identify(std::uint16_t connection_handle, ble_gatt_access_ctxt* context);
    int write_cancel(std::uint16_t connection_handle, ble_gatt_access_ctxt* context);
    bool lock();
    void unlock();
    void clear_session_locked();
    void notify_state(std::uint16_t connection_handle, std::uint8_t state);

    RhophiClaimProtocol& protocol_;
    IClaimGattActions& actions_;
    std::array<std::uint8_t, 32U> response_{};
    SemaphoreHandle_t mutex_ = nullptr;
    std::uint16_t response_value_handle_ = 0U;
    std::uint16_t state_value_handle_ = 0U;
    std::uint16_t claim_connection_handle_ = kNoConnection;
    bool response_valid_ = false;
};

}  // namespace smart_device
