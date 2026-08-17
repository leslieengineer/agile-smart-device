#include "RhophiClaimGatt.hpp"

#include <algorithm>
#include <vector>

#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/BLEManagerImpl.h>

#include "esp_timer.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "os/os_mbuf.h"

namespace smart_device {
namespace {

ble_uuid128_t service_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x10, 0x8e, 0x21, 0x4f, 0x41,
                                              0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t identity_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x11, 0x8e, 0x21, 0x4f, 0x41,
                                               0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t challenge_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x12, 0x8e, 0x21, 0x4f, 0x41,
                                                0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t response_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x13, 0x8e, 0x21, 0x4f, 0x41,
                                               0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t state_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x14, 0x8e, 0x21, 0x4f, 0x41,
                                            0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t identify_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x15, 0x8e, 0x21, 0x4f, 0x41,
                                               0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);
ble_uuid128_t cancel_uuid = BLE_UUID128_INIT(0x9a, 0x7d, 0x52, 0x16, 0x8e, 0x21, 0x4f, 0x41,
                                             0xa1, 0x31, 0x52, 0x48, 0x4f, 0x50, 0x48, 0x49);

ble_gatt_chr_def characteristics[7]{};
ble_gatt_svc_def service{};
RhophiClaimGatt* registered_instance = nullptr;

struct CharacteristicContext {
    RhophiClaimGatt* instance = nullptr;
    RhophiClaimGatt::Characteristic characteristic = RhophiClaimGatt::Characteristic::identity;
};
CharacteristicContext characteristic_contexts[6]{};

std::uint32_t now_ms() {
    return static_cast<std::uint32_t>(esp_timer_get_time() / 1000LL);
}

}  // namespace

RhophiClaimGatt::RhophiClaimGatt(RhophiClaimProtocol& protocol, IClaimGattActions& actions)
    : protocol_{protocol}, actions_{actions} {}

uhal::Status RhophiClaimGatt::initialize() {
    if (registered_instance != nullptr) return uhal::Status::busy;
    mutex_ = xSemaphoreCreateMutex();
    if (mutex_ == nullptr) return uhal::Status::no_resources;

    const ble_uuid_t* uuids[] = {
        &identity_uuid.u, &challenge_uuid.u, &response_uuid.u,
        &state_uuid.u, &identify_uuid.u, &cancel_uuid.u,
    };
    const ble_gatt_chr_flags flags[] = {
        BLE_GATT_CHR_F_READ,
        BLE_GATT_CHR_F_WRITE,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        BLE_GATT_CHR_F_WRITE,
        BLE_GATT_CHR_F_WRITE,
    };
    for (std::size_t index = 0U; index < 6U; ++index) {
        characteristic_contexts[index].instance = this;
        characteristic_contexts[index].characteristic = static_cast<Characteristic>(index);
        characteristics[index] = {};
        characteristics[index].uuid = uuids[index];
        characteristics[index].access_cb = access_callback;
        characteristics[index].arg = &characteristic_contexts[index];
        characteristics[index].flags = flags[index];
    }
    characteristics[2].val_handle = &response_value_handle_;
    characteristics[3].val_handle = &state_value_handle_;
    characteristics[6] = {};

    service = {};
    service.type = BLE_GATT_SVC_TYPE_PRIMARY;
    service.uuid = &service_uuid.u;
    service.characteristics = characteristics;
    std::vector<ble_gatt_svc_def> services{service};
    const CHIP_ERROR error = chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureExtraServices(services, true);
    if (error != CHIP_NO_ERROR) {
        vSemaphoreDelete(mutex_);
        mutex_ = nullptr;
        return uhal::Status::io_error;
    }
    registered_instance = this;
    return uhal::Status::ok;
}

uhal::Status RhophiClaimGatt::open_window(std::uint32_t time_ms, std::uint32_t duration_ms) {
    if (!lock()) return uhal::Status::not_ready;
    clear_session_locked();
    const uhal::Status status = protocol_.open_window(time_ms, duration_ms);
    unlock();
    return status;
}

void RhophiClaimGatt::cancel() {
    if (!lock()) return;
    const std::uint16_t connection = claim_connection_handle_;
    protocol_.cancel();
    clear_session_locked();
    unlock();
    if (connection != kNoConnection) notify_state(connection, 0U);
}

void RhophiClaimGatt::release_session() {
    if (!lock()) return;
    clear_session_locked();
    unlock();
}

uhal::Status RhophiClaimGatt::mark_commissioned() {
    if (!lock()) return uhal::Status::not_ready;
    const std::uint16_t connection = claim_connection_handle_;
    const uhal::Status status = protocol_.mark_commissioned();
    clear_session_locked();
    unlock();
    if (connection != kNoConnection) notify_state(connection, 0U);
    return status;
}

uhal::Status RhophiClaimGatt::factory_reset() {
    if (!lock()) return uhal::Status::not_ready;
    clear_session_locked();
    const uhal::Status status = protocol_.factory_reset();
    unlock();
    return status;
}

int RhophiClaimGatt::access_callback(std::uint16_t connection_handle, std::uint16_t,
                                     ble_gatt_access_ctxt* context, void* argument) {
    auto* characteristic_context = static_cast<CharacteristicContext*>(argument);
    if (context == nullptr || characteristic_context == nullptr ||
        characteristic_context->instance == nullptr) {
        return BLE_ATT_ERR_UNLIKELY;
    }
    return characteristic_context->instance->handle_access(
        characteristic_context->characteristic, connection_handle, context);
}

int RhophiClaimGatt::handle_access(Characteristic characteristic, std::uint16_t connection_handle,
                                   ble_gatt_access_ctxt* context) {
    switch (characteristic) {
        case Characteristic::identity:
            return read_identity(context);
        case Characteristic::challenge:
            return write_challenge(connection_handle, context);
        case Characteristic::response:
            return read_response(connection_handle, context);
        case Characteristic::state:
            return read_state(context);
        case Characteristic::identify:
            return write_identify(connection_handle, context);
        case Characteristic::cancel:
            return write_cancel(connection_handle, context);
    }
    return BLE_ATT_ERR_UNLIKELY;
}

int RhophiClaimGatt::read_identity(ble_gatt_access_ctxt* context) {
    if (!lock()) return BLE_ATT_ERR_UNLIKELY;
    const ClaimIdentity identity = protocol_.identity(now_ms());
    std::array<std::uint8_t, 36U> encoded{};
    encoded[0] = identity.protocol_version;
    encoded[1] = static_cast<std::uint8_t>(identity.product_id & 0xFFU);
    encoded[2] = static_cast<std::uint8_t>(identity.product_id >> 8U);
    std::copy(identity.claim_id.begin(), identity.claim_id.end(), encoded.begin() + 3);
    std::copy(identity.nonce.begin(), identity.nonce.end(), encoded.begin() + 19);
    encoded[35] = identity.flags;
    const int result = os_mbuf_append(context->om, encoded.data(), encoded.size()) == 0
                           ? 0
                           : BLE_ATT_ERR_INSUFFICIENT_RES;
    unlock();
    return result;
}

int RhophiClaimGatt::write_challenge(std::uint16_t connection_handle,
                                     ble_gatt_access_ctxt* context) {
    if (OS_MBUF_PKTLEN(context->om) != response_.size()) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    std::array<std::uint8_t, 32U> challenge{};
    std::uint16_t copied = 0U;
    if (ble_hs_mbuf_to_flat(context->om, challenge.data(), challenge.size(), &copied) != 0 ||
        copied != challenge.size()) {
        return BLE_ATT_ERR_UNLIKELY;
    }
    if (!lock()) {
        std::fill(challenge.begin(), challenge.end(), 0U);
        return BLE_ATT_ERR_UNLIKELY;
    }
    if (claim_connection_handle_ != kNoConnection && claim_connection_handle_ != connection_handle) {
        unlock();
        std::fill(challenge.begin(), challenge.end(), 0U);
        return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    }
    claim_connection_handle_ = connection_handle;
    const uhal::Status status = protocol_.respond(challenge, now_ms(), response_);
    response_valid_ = status == uhal::Status::ok;
    std::array<std::uint8_t, 32U> response = response_;
    unlock();
    std::fill(challenge.begin(), challenge.end(), 0U);
    if (status == uhal::Status::denied) return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    if (status == uhal::Status::busy) return BLE_ATT_ERR_INSUFFICIENT_RES;
    if (status != uhal::Status::ok) return BLE_ATT_ERR_UNLIKELY;

    os_mbuf* notification = ble_hs_mbuf_from_flat(response.data(), response.size());
    std::fill(response.begin(), response.end(), 0U);
    if (notification == nullptr) return BLE_ATT_ERR_INSUFFICIENT_RES;
    const int notify_result = ble_gatts_notify_custom(
        connection_handle, response_value_handle_, notification);
    notify_state(connection_handle, 1U);
    return notify_result == 0 ? 0 : BLE_ATT_ERR_UNLIKELY;
}

int RhophiClaimGatt::read_response(std::uint16_t connection_handle,
                                   ble_gatt_access_ctxt* context) {
    if (!lock()) return BLE_ATT_ERR_UNLIKELY;
    if (claim_connection_handle_ != connection_handle || !response_valid_) {
        unlock();
        return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    }
    const int result = os_mbuf_append(context->om, response_.data(), response_.size()) == 0
                           ? 0
                           : BLE_ATT_ERR_INSUFFICIENT_RES;
    unlock();
    return result;
}

int RhophiClaimGatt::read_state(ble_gatt_access_ctxt* context) {
    if (!lock()) return BLE_ATT_ERR_UNLIKELY;
    const std::uint8_t active = protocol_.is_active(now_ms()) ? 1U : 0U;
    const int result = os_mbuf_append(context->om, &active, sizeof(active)) == 0
                           ? 0
                           : BLE_ATT_ERR_INSUFFICIENT_RES;
    unlock();
    return result;
}

int RhophiClaimGatt::write_identify(std::uint16_t, ble_gatt_access_ctxt* context) {
    if (OS_MBUF_PKTLEN(context->om) != 0U) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    if (!lock()) return BLE_ATT_ERR_UNLIKELY;
    const bool active = protocol_.is_active(now_ms());
    unlock();
    if (!active) return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    actions_.identify_claim_device();
    return 0;
}

int RhophiClaimGatt::write_cancel(std::uint16_t connection_handle,
                                  ble_gatt_access_ctxt* context) {
    if (OS_MBUF_PKTLEN(context->om) != 0U) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    if (!lock()) return BLE_ATT_ERR_UNLIKELY;
    if (claim_connection_handle_ != connection_handle) {
        unlock();
        return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    }
    protocol_.cancel();
    clear_session_locked();
    unlock();
    notify_state(connection_handle, 0U);
    return 0;
}

bool RhophiClaimGatt::lock() {
    return mutex_ != nullptr && xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE;
}

void RhophiClaimGatt::unlock() {
    xSemaphoreGive(mutex_);
}

void RhophiClaimGatt::clear_session_locked() {
    claim_connection_handle_ = kNoConnection;
    response_valid_ = false;
    std::fill(response_.begin(), response_.end(), 0U);
}

void RhophiClaimGatt::notify_state(std::uint16_t connection_handle, std::uint8_t state) {
    if (state_value_handle_ == 0U) return;
    os_mbuf* notification = ble_hs_mbuf_from_flat(&state, sizeof(state));
    if (notification != nullptr) {
        (void)ble_gatts_notify_custom(connection_handle, state_value_handle_, notification);
    }
}

}  // namespace smart_device
