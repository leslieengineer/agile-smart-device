#pragma once
#include <libraries/Backoff.hpp>
#include <services/INetworkLink.hpp>
#include <services/Limits.hpp>
namespace services
{
class NetworkManager final
{
public:
    uhal::Status      add_link(INetworkLink& link);
    void              set_observer(ILinkObserver* observer);
    void              poll(std::uint32_t now_ms);
    ConnectivityState state() const;
    LinkKind          active_kind() const;

private:
    void                     publish(ConnectivityState state, std::size_t index);
    INetworkLink*            links_[kNetworkLinkCapacity]{};
    libraries::BackoffPolicy backoff_[kNetworkLinkCapacity]{};
    std::uint32_t            retry_at_[kNetworkLinkCapacity]{};
    std::size_t              count_    = 0U;
    std::size_t              active_   = kNetworkLinkCapacity;
    ConnectivityState        state_    = ConnectivityState::provisioning_required;
    ILinkObserver*           observer_ = nullptr;
};
}  // namespace services
