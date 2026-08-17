#include "SwitchRuntime.hpp"

#include <board/BoardPins.hpp>

#include "esp_log.h"

namespace smart_device {
namespace {
constexpr char          kTag[]        = "switch-runtime";
constexpr std::uint32_t kPollMs       = 5U;
constexpr std::uint32_t kTaskStack    = 3072U;
constexpr UBaseType_t   kTaskPriority = 5U;
constexpr UBaseType_t   kQueueDepth   = 8U;
}  // namespace

SwitchRuntime::SwitchRuntime(board::Board& board, SmartDeviceApplication& application,
                             INodeLifecycleActions* lifecycle)
    : board_{board}, application_{application}, lifecycle_{lifecycle} {}

uhal::Status SwitchRuntime::start() {
    queue_ = xQueueCreate(kQueueDepth, sizeof(Event));
    if (queue_ == nullptr) return uhal::Status::io_error;
    if (xTaskCreate(task_entry, "switch_ctrl", kTaskStack, this, kTaskPriority, &task_) != pdPASS) {
        vQueueDelete(queue_);
        queue_ = nullptr;
        return uhal::Status::io_error;
    }
    uhal::Status status =
        board_.button_interrupt().attach(uhal::InterruptTrigger::both_edges, button_isr, this);
    if (status == uhal::Status::ok) status = board_.button_interrupt().enable();
    if (status != uhal::Status::ok) {
        vTaskDelete(task_);
        vQueueDelete(queue_);
        task_  = nullptr;
        queue_ = nullptr;
    }
    return status;
}

uhal::Status SwitchRuntime::post_set_switch(bool on) {
    if (queue_ == nullptr) return uhal::Status::not_ready;
    const Event event{EventType::set_switch, on};
    return xQueueSend(queue_, &event, 0U) == pdPASS ? uhal::Status::ok : uhal::Status::busy;
}

void SwitchRuntime::button_isr(void* context) {
    auto*      runtime = static_cast<SwitchRuntime*>(context);
    const Event event{EventType::button_edge, false};
    BaseType_t higher_priority_woken = pdFALSE;
    xQueueSendFromISR(runtime->queue_, &event, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

void SwitchRuntime::task_entry(void* context) {
    static_cast<SwitchRuntime*>(context)->run();
}

void SwitchRuntime::run() {
    for (;;) {
        const TickType_t wait = button_input_.is_active() ? pdMS_TO_TICKS(kPollMs) : portMAX_DELAY;
        Event            event{};
        if (xQueueReceive(queue_, &event, wait) == pdPASS && event.type == EventType::set_switch) {
            const uhal::Status status = application_.set_switch(event.on);
            if (status != uhal::Status::ok) ESP_LOGE(kTag, "Remote switch command failed");
        }
        sample_button();
    }
}

void SwitchRuntime::sample_button() {
    uhal::GpioLevel level{};
    if (board_.button().get(level) != uhal::Status::ok) return;
    const bool pressed = board::kButtonActiveLow ? level == uhal::GpioLevel::low
                                                 : level == uhal::GpioLevel::high;
    const std::uint32_t now_ms = board_.clock().now_ms();
    const std::uint32_t held_ms = button_input_.hold_ms(now_ms);
    const libraries::ButtonEvent event = button_input_.update(pressed, now_ms);
    if (event == libraries::ButtonEvent::commissioning_press ||
        event == libraries::ButtonEvent::factory_reset_press) {
        ESP_LOGI(kTag, "Button event=%u held_ms=%u", static_cast<unsigned>(event), held_ms);
    }
    uhal::Status status = uhal::Status::ok;
    if (event == libraries::ButtonEvent::short_press) {
        status = application_.on_short_press();
    } else if (event == libraries::ButtonEvent::commissioning_press && lifecycle_ != nullptr) {
        status = lifecycle_->open_commissioning_window();
    } else if (event == libraries::ButtonEvent::factory_reset_press && lifecycle_ != nullptr) {
        status = lifecycle_->factory_reset();
    }
    if (status != uhal::Status::ok) ESP_LOGE(kTag, "Button action failed");
}

}  // namespace smart_device
