#include <services/ProvisioningService.hpp>
namespace services
{
ProvisioningService::ProvisioningService(IProvisioningPortal& p, IProvisioningValidator& v,
                                         IConfigStore& s, PortalConfig c)
    : portal_{p}, validator_{v}, store_{s}, config_{c}
{
}
uhal::Status ProvisioningService::start(std::uint32_t now)
{
    const uhal::Status s = portal_.open(config_);
    if (s == uhal::Status::ok)
    {
        state_     = ProvisioningState::portal_open;
        opened_at_ = now;
    }
    return s;
}
uhal::Status ProvisioningService::stop()
{
    clear_pending();
    const uhal::Status s = portal_.close();
    if (s == uhal::Status::ok && state_ != ProvisioningState::provisioned)
        state_ = ProvisioningState::unprovisioned;
    return s;
}
void ProvisioningService::poll(std::uint32_t now)
{
    portal_.poll(now);
    if (state_ == ProvisioningState::portal_open && now - opened_at_ >= config_.timeout_ms)
    {
        stop();
        return;
    }
    if (state_ == ProvisioningState::portal_open && portal_.has_credentials())
    {
        if (portal_.take_credentials(pending_) == uhal::Status::ok)
            state_ = ProvisioningState::validating;
    }
    if (state_ != ProvisioningState::validating)
        return;
    const uhal::Status validation = validator_.validate(pending_);
    if (validation == uhal::Status::busy)
        return;
    if (validation == uhal::Status::ok)
    {
        const uhal::Status stored = store_.set_blob(
            "net.credentials", reinterpret_cast<const std::uint8_t*>(&pending_), sizeof(pending_));
        if (stored == uhal::Status::ok && store_.commit() == uhal::Status::ok)
        {
            clear_pending();
            portal_.close();
            state_    = ProvisioningState::provisioned;
            failures_ = 0U;
            return;
        }
    }
    fail_attempt();
}
uhal::Status ProvisioningService::factory_reset()
{
    const uhal::Status s = store_.erase("net.credentials");
    if (s != uhal::Status::ok && s != uhal::Status::not_found)
        return s;
    const uhal::Status c = store_.commit();
    if (c == uhal::Status::ok)
    {
        clear_pending();
        state_    = ProvisioningState::unprovisioned;
        failures_ = 0U;
    }
    return c;
}
ProvisioningState ProvisioningService::state() const
{
    return state_;
}
void ProvisioningService::fail_attempt()
{
    clear_pending();
    ++failures_;
    state_ = failures_ >= 3U ? ProvisioningState::failed : ProvisioningState::portal_open;
}
void ProvisioningService::clear_pending()
{
    volatile std::uint8_t* data = reinterpret_cast<volatile std::uint8_t*>(&pending_);
    for (std::size_t i = 0U; i < sizeof(pending_); ++i) data[i] = 0U;
}
}  // namespace services
