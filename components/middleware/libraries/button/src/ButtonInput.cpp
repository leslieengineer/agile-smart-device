#include <libraries/ButtonInput.hpp>

namespace libraries
{

ButtonInput::ButtonInput(ButtonInputConfig config) : config_{config} {}

ButtonEvent ButtonInput::update(bool raw_pressed, std::uint32_t now_ms)
{
    if (raw_pressed != raw_last_)
    {
        raw_last_       = raw_pressed;
        debouncing_     = true;
        last_change_ms_ = now_ms;
        return ButtonEvent::none;
    }
    if (!debouncing_ || (now_ms - last_change_ms_) < config_.debounce_ms)
    {
        return ButtonEvent::none;
    }
    debouncing_ = false;
    if (stable_pressed_ == raw_last_)
    {
        return ButtonEvent::none;
    }
    stable_pressed_ = raw_last_;
    if (stable_pressed_)
    {
        press_start_ms_ = now_ms;
        return ButtonEvent::press_started;
    }
    const std::uint32_t held_ms = now_ms - press_start_ms_;
    if (held_ms >= config_.factory_reset_press_min_ms)
        return ButtonEvent::factory_reset_press;
    if (held_ms >= config_.commissioning_press_min_ms)
        return ButtonEvent::commissioning_press;
    return held_ms <= config_.short_press_max_ms ? ButtonEvent::short_press : ButtonEvent::none;
}

bool ButtonInput::is_pressed() const
{
    return stable_pressed_;
}
bool ButtonInput::is_active() const
{
    return debouncing_ || stable_pressed_;
}
std::uint32_t ButtonInput::hold_ms(std::uint32_t now_ms) const
{
    return stable_pressed_ ? now_ms - press_start_ms_ : 0U;
}

}  // namespace libraries
