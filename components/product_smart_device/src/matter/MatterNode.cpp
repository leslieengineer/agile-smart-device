#include "MatterNode.hpp"

#include <board/BoardPins.hpp>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/OpenthreadLauncher.h>

#include "esp_log.h"
#include "esp_openthread_types.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "led_strip_rmt.h"

namespace smart_device {
namespace {

constexpr char kTag[] = "matter-node";
constexpr auto          kCommissioningWindowTimeout = chip::System::Clock::Seconds16(900U);
constexpr std::uint32_t kClaimWindowDurationMs = 900000U;

uhal::Status status_from_esp(esp_err_t error) {
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

}  // namespace

void MatterNode::bind_runtime(SwitchRuntime& runtime) {
    runtime_ = &runtime;
}

uhal::Status MatterNode::start() {
    using namespace esp_matter;
    using namespace esp_matter::endpoint;

    if (runtime_ == nullptr) return uhal::Status::not_ready;
    const uhal::Status claim_status = claim_protocol_.initialize();
    if (claim_status != uhal::Status::ok) {
        ESP_LOGW(kTag, "Rhophi claim material unavailable; mobile claim service disabled");
    } else if (claim_gatt_.initialize() != uhal::Status::ok) {
        ESP_LOGW(kTag, "Rhophi claim GATT service unavailable");
    } else {
        claim_ready_ = true;
#ifdef RHOPHI_CLAIM_DEV_BYPASS
        ESP_LOGW(kTag, "DEVELOPMENT claim bypass enabled; ownership proof is not verified");
#else
        ESP_LOGI(kTag, "Rhophi claim service ready");
#endif
    }
    if (initialize_indicator() == uhal::Status::ok) {
        set_indicator(24U, 12U, 0U);
    } else {
        ESP_LOGW(kTag, "WS2812 status indicator unavailable");
    }

    node::config_t node_config{};
    node_t* matter_node = node::create(&node_config, attribute_update_callback, identification_callback);
    if (matter_node == nullptr) return uhal::Status::no_resources;

    on_off_plug_in_unit::config_t plug_config{};
    plug_config.on_off.on_off = pending_state_.load();
    endpoint_t* plug_endpoint =
        on_off_plug_in_unit::create(matter_node, &plug_config, ENDPOINT_FLAG_NONE, this);
    if (plug_endpoint == nullptr) return uhal::Status::no_resources;

    endpoint_id_ = endpoint::get_id(plug_endpoint);
    ESP_LOGI(kTag, "On/Off Plug-in Unit created on endpoint %u", endpoint_id_);
    ESP_LOGI(kTag, "reset_reason=%d free_heap=%u min_free_heap=%u", esp_reset_reason(),
             esp_get_free_heap_size(), esp_get_minimum_free_heap_size());

    esp_openthread_platform_config_t thread_config{};
    thread_config.radio_config.radio_mode = RADIO_MODE_NATIVE;
    thread_config.host_config.host_connection_mode = HOST_CONNECTION_MODE_NONE;
    thread_config.port_config.storage_partition_name = "nvs";
    thread_config.port_config.netif_queue_size = 10U;
    thread_config.port_config.task_queue_size = 10U;
    set_openthread_platform_config(&thread_config);

    const esp_err_t error =
        esp_matter::start(device_event_callback, reinterpret_cast<intptr_t>(this));
    if (error != ESP_OK) return status_from_esp(error);

    started_ = true;
    schedule_state_report();
    return uhal::Status::ok;
}

void MatterNode::on_switch_state_changed(bool on) {
    pending_state_.store(on);
    if (started_) schedule_state_report();
}

void MatterNode::identify_claim_device() {
    if (!started_) return;
    const CHIP_ERROR error = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        identify_work, reinterpret_cast<intptr_t>(this));
    if (error != CHIP_NO_ERROR) {
        ESP_LOGW(kTag, "Failed to schedule claim identification");
    }
}

uhal::Status MatterNode::open_commissioning_window() {
    if (!started_ || !claim_ready_) return uhal::Status::not_ready;
    const std::uint32_t now_ms = static_cast<std::uint32_t>(esp_timer_get_time() / 1000LL);
    const uhal::Status claim_status = claim_gatt_.open_window(now_ms, kClaimWindowDurationMs);
    if (claim_status != uhal::Status::ok) {
        ESP_LOGW(kTag, "Failed to open Rhophi claim window");
        return claim_status;
    }
    const CHIP_ERROR error = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        open_commissioning_work, reinterpret_cast<intptr_t>(this));
    if (error != CHIP_NO_ERROR) claim_gatt_.cancel();
    return error == CHIP_NO_ERROR ? uhal::Status::ok : uhal::Status::busy;
}

uhal::Status MatterNode::factory_reset() {
    if (!started_) return uhal::Status::not_ready;
    if (claim_ready_ && claim_gatt_.factory_reset() != uhal::Status::ok) {
        return uhal::Status::io_error;
    }
    return status_from_esp(esp_matter::factory_reset());
}

esp_err_t MatterNode::attribute_update_callback(esp_matter::attribute::callback_type_t type,
                                                std::uint16_t endpoint_id,
                                                std::uint32_t cluster_id,
                                                std::uint32_t attribute_id,
                                                esp_matter_attr_val_t* value,
                                                void* private_data) {
    auto* self = static_cast<MatterNode*>(private_data);
    if (self == nullptr || value == nullptr || self->runtime_ == nullptr) return ESP_ERR_INVALID_ARG;
    if (type != esp_matter::attribute::PRE_UPDATE || self->applying_report_) return ESP_OK;
    if (endpoint_id != self->endpoint_id_ || cluster_id != chip::app::Clusters::OnOff::Id ||
        attribute_id != chip::app::Clusters::OnOff::Attributes::OnOff::Id) {
        return ESP_OK;
    }

    const uhal::Status status = self->runtime_->post_set_switch(value->val.b);
    if (status == uhal::Status::ok) return ESP_OK;
    return status == uhal::Status::busy ? ESP_ERR_NO_MEM : ESP_ERR_INVALID_STATE;
}

esp_err_t MatterNode::identification_callback(esp_matter::identification::callback_type_t type,
                                              std::uint16_t endpoint_id,
                                              std::uint8_t effect_id,
                                              std::uint8_t effect_variant,
                                              void* private_data) {
    auto* self = static_cast<MatterNode*>(private_data);
    ESP_LOGI(kTag, "Identify type=%u endpoint=%u effect=%u variant=%u", static_cast<unsigned>(type),
             endpoint_id, effect_id, effect_variant);
    if (self != nullptr) self->set_indicator(24U, 24U, 24U);
    return ESP_OK;
}

void MatterNode::device_event_callback(const ChipDeviceEvent* event, intptr_t private_data) {
    auto* self = reinterpret_cast<MatterNode*>(private_data);
    if (self == nullptr || event == nullptr) return;

    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ESP_LOGI(kTag, "Commissioning complete");
            if (self->claim_ready_ && self->claim_gatt_.mark_commissioned() != uhal::Status::ok) {
                ESP_LOGE(kTag, "Failed to persist completed claim state");
            }
            self->set_indicator(0U, 20U, 0U);
            break;
        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
            ESP_LOGI(kTag, "Commissioning window opened");
            if (self->claim_ready_ &&
                chip::Server::GetInstance().GetFabricTable().FabricCount() == 0U) {
                const std::uint32_t now_ms =
                    static_cast<std::uint32_t>(esp_timer_get_time() / 1000LL);
                const uhal::Status status =
                    self->claim_gatt_.open_window(now_ms, kClaimWindowDurationMs);
                if (status == uhal::Status::ok) {
                    ESP_LOGI(kTag, "Rhophi claim window opened");
                } else {
                    ESP_LOGW(kTag, "Failed to open Rhophi claim window with Matter window");
                }
            }
            self->set_indicator(24U, 12U, 0U);
            break;
        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
            ESP_LOGI(kTag, "Commissioning window closed");
            if (self->claim_ready_) self->claim_gatt_.cancel();
            self->set_indicator(0U, 0U, 8U);
            break;
        case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
            ESP_LOGW(kTag, "Commissioning fail-safe timer expired");
            if (self->claim_ready_) self->claim_gatt_.release_session();
            self->set_indicator(24U, 0U, 0U);
            break;
        case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
            if (self->claim_ready_) self->claim_gatt_.release_session();
            break;
        case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
            ESP_LOGI(kTag, "Fabric removed");
            self->set_indicator(24U, 12U, 0U);
            break;
        case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
            ESP_LOGI(kTag, "Thread interface address changed");
            self->set_indicator(0U, 0U, 12U);
            break;
        default:
            break;
    }
}

void MatterNode::schedule_state_report() {
    if (report_scheduled_.exchange(true)) return;
    const CHIP_ERROR error = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        report_state_work, reinterpret_cast<intptr_t>(this));
    if (error != CHIP_NO_ERROR) {
        report_scheduled_.store(false);
        ESP_LOGE(kTag, "Failed to schedule OnOff report: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void MatterNode::report_state_work(intptr_t private_data) {
    auto* self = reinterpret_cast<MatterNode*>(private_data);
    if (self != nullptr) self->report_state();
}

void MatterNode::identify_work(intptr_t private_data) {
    auto* self = reinterpret_cast<MatterNode*>(private_data);
    if (self != nullptr) self->set_indicator(24U, 24U, 24U);
}

void MatterNode::report_state() {
    const bool state = pending_state_.load();
    esp_matter_attr_val_t value = esp_matter_bool(state);
    applying_report_ = true;
    const esp_err_t error = esp_matter::attribute::update(
        endpoint_id_, chip::app::Clusters::OnOff::Id,
        chip::app::Clusters::OnOff::Attributes::OnOff::Id, &value);
    applying_report_ = false;
    report_scheduled_.store(false);

    if (error != ESP_OK) ESP_LOGE(kTag, "Failed to publish OnOff=%d: %d", state, error);
    if (pending_state_.load() != state) schedule_state_report();
}

void MatterNode::open_commissioning_work(intptr_t private_data) {
    auto* self = reinterpret_cast<MatterNode*>(private_data);
    if (self == nullptr || !self->started_) return;

    auto& manager = chip::Server::GetInstance().GetCommissioningWindowManager();
    if (manager.IsCommissioningWindowOpen()) return;
    const CHIP_ERROR error = manager.OpenBasicCommissioningWindow(
        kCommissioningWindowTimeout, chip::CommissioningWindowAdvertisement::kAllSupported);
    if (error != CHIP_NO_ERROR) {
        self->claim_gatt_.cancel();
        ESP_LOGE(kTag, "Failed to open commissioning window: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

uhal::Status MatterNode::initialize_indicator() {
    led_strip_config_t strip_config{};
    strip_config.strip_gpio_num = board::kStatusPixelPin;
    strip_config.max_leds = 1U;
    strip_config.led_model = LED_MODEL_WS2812;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.flags.invert_out = false;

    led_strip_rmt_config_t rmt_config{};
    rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;
    rmt_config.resolution_hz = 10000000U;
    rmt_config.mem_block_symbols = 64U;
    rmt_config.flags.with_dma = false;

    return led_strip_new_rmt_device(&strip_config, &rmt_config, &indicator_) == ESP_OK
               ? uhal::Status::ok
               : uhal::Status::io_error;
}

void MatterNode::set_indicator(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    if (indicator_ == nullptr) return;
    if (led_strip_set_pixel(indicator_, 0U, red, green, blue) != ESP_OK ||
        led_strip_refresh(indicator_) != ESP_OK) {
        ESP_LOGE(kTag, "Failed to update WS2812 status indicator");
    }
}

}  // namespace smart_device
