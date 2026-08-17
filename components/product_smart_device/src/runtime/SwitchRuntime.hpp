#pragma once

#include <cstdint>

#include <board/Board.hpp>
#include <libraries/ButtonInput.hpp>
#include <smart_device/SmartDeviceApplication.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace smart_device {

class INodeLifecycleActions {
public:
    virtual ~INodeLifecycleActions() = default;
    virtual uhal::Status open_commissioning_window() = 0;
    virtual uhal::Status factory_reset() = 0;
};

class SwitchRuntime final {
public:
    SwitchRuntime(board::Board& board, SmartDeviceApplication& application,
                  INodeLifecycleActions* lifecycle = nullptr);
    uhal::Status start();
    uhal::Status post_set_switch(bool on);

private:
    enum class EventType : std::uint8_t { button_edge, set_switch };

    struct Event {
        EventType type = EventType::button_edge;
        bool      on   = false;
    };

    static void button_isr(void* context);
    static void task_entry(void* context);
    void        run();
    void        sample_button();

    board::Board&           board_;
    SmartDeviceApplication& application_;
    INodeLifecycleActions*  lifecycle_ = nullptr;
    libraries::ButtonInput  button_input_{libraries::ButtonInputConfig{25U, 1000U, 3000U, 10000U}};
    QueueHandle_t           queue_ = nullptr;
    TaskHandle_t            task_  = nullptr;
};

}  // namespace smart_device
