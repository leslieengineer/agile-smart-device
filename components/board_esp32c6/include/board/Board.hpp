#pragma once

#include <uhal/Status.hpp>

#include "Esp32C6Clock.hpp"
#include "Esp32C6Gpio.hpp"

namespace board {

class Board final {
public:
    Board();

    uhal::Status          initialize();
    uhal::IGpio&          relay();
    uhal::IGpio&          led();
    uhal::IGpio&          button();
    uhal::IGpioInterrupt& button_interrupt();
    uhal::IClock&         clock();

private:
    esp32c6::adapters::OutputPin    relay_;
    esp32c6::adapters::OutputPin    led_;
    esp32c6::adapters::InputPin     button_;
    esp32c6::adapters::PinInterrupt button_interrupt_;
    esp32c6::adapters::Clock        clock_;
};

}  // namespace board
