#pragma once

#include <atomic>
#include <cstdint>

#include <smart_device/SmartDeviceApplication.hpp>

#include "NvsClaimStateStore.hpp"
#include "RhophiClaimGatt.hpp"
#include "RhophiClaimPlatform.hpp"
#include "SwitchRuntime.hpp"
#include "esp_matter.h"
#include "led_strip.h"

namespace smart_device {

class MatterNode final : public ISwitchStateObserver, public INodeLifecycleActions, public IClaimGattActions {
public:
    void         bind_runtime(SwitchRuntime& runtime);
    uhal::Status start();

    void         on_switch_state_changed(bool on) override;
    uhal::Status open_commissioning_window() override;
    uhal::Status factory_reset() override;
    void         identify_claim_device() override;

private:
    static esp_err_t attribute_update_callback(esp_matter::attribute::callback_type_t type,
                                               std::uint16_t endpoint_id, std::uint32_t cluster_id,
                                               std::uint32_t attribute_id,
                                               esp_matter_attr_val_t* value, void* private_data);
    static esp_err_t identification_callback(esp_matter::identification::callback_type_t type,
                                             std::uint16_t endpoint_id, std::uint8_t effect_id,
                                             std::uint8_t effect_variant, void* private_data);
    static void      device_event_callback(const ChipDeviceEvent* event, intptr_t private_data);
    static void      report_state_work(intptr_t private_data);
    static void      open_commissioning_work(intptr_t private_data);
    static void      identify_work(intptr_t private_data);

    void         schedule_state_report();
    void         report_state();
    uhal::Status initialize_indicator();
    void         set_indicator(std::uint8_t red, std::uint8_t green, std::uint8_t blue);

    NvsClaimMaterialProvider claim_material_provider_{};
    EspClaimCrypto           claim_crypto_{};
    NvsClaimStateStore       claim_state_store_{};
    RhophiClaimProtocol      claim_protocol_{claim_material_provider_, claim_crypto_, claim_state_store_};
    RhophiClaimGatt          claim_gatt_{claim_protocol_, *this};
    SwitchRuntime*           runtime_ = nullptr;
    led_strip_handle_t       indicator_ = nullptr;
    std::uint16_t          endpoint_id_ = 0U;
    std::atomic_bool       pending_state_{false};
    std::atomic_bool       report_scheduled_{false};
    bool                   started_ = false;
    bool                   applying_report_ = false;
    bool                   claim_ready_ = false;
};

}  // namespace smart_device
