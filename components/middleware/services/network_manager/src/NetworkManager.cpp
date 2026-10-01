#include <services/NetworkManager.hpp>
namespace services
{
uhal::Status NetworkManager::add_link(INetworkLink& link)
{
    if (count_ >= kNetworkLinkCapacity)
        return uhal::Status::no_resources;
    links_[count_++] = &link;
    if (state_ == ConnectivityState::provisioning_required)
        state_ = ConnectivityState::offline;
    return uhal::Status::ok;
}
void NetworkManager::set_observer(ILinkObserver* observer)
{
    observer_ = observer;
}
void NetworkManager::poll(std::uint32_t now_ms)
{
    for (std::size_t i = 0U; i < count_; ++i) links_[i]->poll(now_ms);
    for (std::size_t i = 0U; i < count_; ++i)
    {
        if (links_[i]->state() == LinkState::up)
        {
            active_ = i;
            backoff_[i].reset();
            publish(i == 0U ? ConnectivityState::online_primary : ConnectivityState::online_backup,
                    i);
            return;
        }
    }
    active_        = kNetworkLinkCapacity;
    bool attempted = false;
    for (std::size_t i = 0U; i < count_; ++i)
    {
        if (backoff_[i].exhausted() || static_cast<std::int32_t>(now_ms - retry_at_[i]) < 0)
            continue;
        const uhal::Status status = links_[i]->start();
        retry_at_[i]              = now_ms + backoff_[i].next_delay_ms();
        attempted                 = true;
        if (status == uhal::Status::ok || status == uhal::Status::busy)
            break;
    }
    publish(attempted ? ConnectivityState::connecting : ConnectivityState::offline, 0U);
}
ConnectivityState NetworkManager::state() const
{
    return state_;
}
LinkKind NetworkManager::active_kind() const
{
    return active_ < count_ ? links_[active_]->kind() : LinkKind::wifi;
}
void NetworkManager::publish(ConnectivityState state, std::size_t index)
{
    if (state_ == state)
        return;
    state_ = state;
    if (observer_ != nullptr && index < count_)
        observer_->on_connectivity_changed(state, links_[index]->kind(), links_[index]->quality());
}
}  // namespace services
