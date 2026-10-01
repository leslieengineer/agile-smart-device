#include <services/BinarySwitchService.hpp>

namespace services
{

BinarySwitchService::BinarySwitchService(uhal::IGpio& load_output, uhal::IGpio& indicator,
                                         IBinaryStateStore& store)
    : load_output_{load_output}, indicator_{indicator}, store_{store}
{
}

uhal::Status BinarySwitchService::restore()
{
    BinaryState restored{};
    if (store_.load(restored) == uhal::Status::ok)
    {
        state_ = restored;
    }
    else
    {
        state_.on = false;
    }
    return apply();
}

uhal::Status BinarySwitchService::set(bool on)
{
    state_.on                       = on;
    const uhal::Status apply_status = apply();
    if (apply_status != uhal::Status::ok)
    {
        return apply_status;
    }
    return store_.save(state_);
}

uhal::Status BinarySwitchService::toggle()
{
    return set(!state_.on);
}

bool BinarySwitchService::is_on() const
{
    return state_.on;
}

uhal::Status BinarySwitchService::apply()
{
    const uhal::GpioLevel level       = state_.on ? uhal::GpioLevel::high : uhal::GpioLevel::low;
    const uhal::Status    load_status = load_output_.set(level);
    const uhal::Status    indicator_status = indicator_.set(level);
    return load_status == uhal::Status::ok && indicator_status == uhal::Status::ok
               ? uhal::Status::ok
               : uhal::Status::io_error;
}

}  // namespace services
