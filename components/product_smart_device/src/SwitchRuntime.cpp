#include "SwitchRuntime.hpp"

#include <board/BoardPins.hpp>

#include "esp_log.h"

namespace smart_device {
namespace {
constexpr char          kTag[]        = "switch-runtime";
constexpr std::uint32_t kPollMs       = 5U;
constexpr std::uint32_t kTaskStack    = 3072U;
constexpr UBaseType_t   kTaskPriority = 5U;
}  // namespace

SwitchRuntime::SwitchRuntime(board::Board& board, SmartDeviceApplication& application)
    : board_{board}, application_{application} {}

uhal::Status SwitchRuntime::start() {
    queue_ = xQueueCreate(1U, sizeof(std::uint8_t));
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

void SwitchRuntime::button_isr(void* context) {
    auto*        runtime               = static_cast<SwitchRuntime*>(context);
    std::uint8_t token                 = 1U;
    BaseType_t   higher_priority_woken = pdFALSE;
    xQueueOverwriteFromISR(runtime->queue_, &token, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

void SwitchRuntime::task_entry(void* context) {
    static_cast<SwitchRuntime*>(context)->run();
}

void SwitchRuntime::run() {
    for (;;) {
        const TickType_t wait = button_input_.is_active() ? pdMS_TO_TICKS(kPollMs) : portMAX_DELAY;
        std::uint8_t     token{};
        xQueueReceive(queue_, &token, wait);
        uhal::GpioLevel level{};
        if (board_.button().get(level) != uhal::Status::ok) continue;
        const bool pressed = board::kButtonActiveLow ? level == uhal::GpioLevel::low
                                                     : level == uhal::GpioLevel::high;
        const libraries::ButtonEvent event = button_input_.update(pressed, board_.clock().now_ms());
        if (event == libraries::ButtonEvent::short_press) {
            const uhal::Status status = application_.on_short_press();
            if (status != uhal::Status::ok) ESP_LOGE(kTag, "Switch command failed");
        }
    }
}

}  // namespace smart_device
