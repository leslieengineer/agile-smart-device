#pragma once
#include <cstdint>
#include <uhal/IGpio.hpp>
#include <uhal/IGpioInterrupt.hpp>
namespace esp32c6::adapters
{
class OutputPin final : public uhal::IGpio
{
public:
    OutputPin(std::uint8_t pin, bool active_low, uhal::GpioLevel initial_level);
    uhal::Status set(uhal::GpioLevel level) override;
    uhal::Status get(uhal::GpioLevel& level) override;

private:
    std::uint8_t    pin_;
    bool            active_low_;
    bool            valid_;
    uhal::GpioLevel level_;
};
class InputPin final : public uhal::IGpio
{
public:
    InputPin(std::uint8_t pin, bool pull_up, bool pull_down = false);
    uhal::Status set(uhal::GpioLevel level) override;
    uhal::Status get(uhal::GpioLevel& level) override;

private:
    std::uint8_t pin_;
    bool         valid_;
};
class PinInterrupt final : public uhal::IGpioInterrupt
{
public:
    explicit PinInterrupt(std::uint8_t pin);
    ~PinInterrupt() override;
    uhal::Status attach(uhal::InterruptTrigger trigger, Callback callback, void* context) override;
    uhal::Status detach() override;
    uhal::Status enable() override;
    uhal::Status disable() override;

private:
    std::uint8_t pin_;
    bool         attached_ = false;
};
}  // namespace esp32c6::adapters
