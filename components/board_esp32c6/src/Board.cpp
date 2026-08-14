#include <board/Board.hpp>
#include <board/BoardPins.hpp>

namespace board {

Board::Board()
    : relay_{kRelayPin, kRelayActiveLow, uhal::GpioLevel::low},
      led_{kLedPin, kLedActiveLow, uhal::GpioLevel::low},
      button_{kButtonPin, true},
      button_interrupt_{kButtonPin} {}

uhal::Status Board::initialize() {
    if (relay_.set(uhal::GpioLevel::low) != uhal::Status::ok ||
        led_.set(uhal::GpioLevel::low) != uhal::Status::ok) {
        return uhal::Status::io_error;
    }
    uhal::GpioLevel level{};
    return button_.get(level);
}

uhal::IGpio& Board::relay() {
    return relay_;
}
uhal::IGpio& Board::led() {
    return led_;
}
uhal::IGpio& Board::button() {
    return button_;
}
uhal::IGpioInterrupt& Board::button_interrupt() {
    return button_interrupt_;
}
uhal::IClock& Board::clock() {
    return clock_;
}

}  // namespace board
