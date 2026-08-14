#pragma once

#include <board/Board.hpp>
#include <libraries/ButtonInput.hpp>
#include <smart_device/SwitchController.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace smart_device {

class SwitchRuntime final {
public:
    SwitchRuntime(board::Board& board, SwitchController& controller);
    uhal::Status start();

private:
    static void button_isr(void* context);
    static void task_entry(void* context);
    void        run();

    board::Board&          board_;
    SwitchController&      controller_;
    libraries::ButtonInput button_input_{};
    QueueHandle_t          queue_ = nullptr;
    TaskHandle_t           task_  = nullptr;
};

}  // namespace smart_device
