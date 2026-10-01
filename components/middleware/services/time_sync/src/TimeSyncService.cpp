#include <services/TimeSyncService.hpp>
namespace services
{
TimeSyncService::TimeSyncService(ITimeSource& s, ISystemTime& t) : source_{s}, system_{t} {}
void TimeSyncService::on_connectivity_up(std::uint32_t now)
{
    retry_at_  = now;
    requested_ = false;
}
void TimeSyncService::poll(std::uint32_t now)
{
    source_.poll(now);
    if (requested_ && source_.has_result())
    {
        std::uint32_t      utc    = 0U;
        const uhal::Status result = source_.take_utc(utc);
        requested_                = false;
        if (result == uhal::Status::ok && utc > 1577836800U)
        {
            system_.set_utc(utc);
            quality_      = TimeQuality::synced;
            last_sync_ms_ = now;
            backoff_.reset();
            retry_at_ = now + 21600000U;
        }
        else
            retry_at_ = now + backoff_.next_delay_ms();
    }
    if (quality_ == TimeQuality::synced && now - last_sync_ms_ >= 86400000U)
        quality_ = TimeQuality::stale;
    if (!requested_ && static_cast<std::int32_t>(now - retry_at_) >= 0)
    {
        const uhal::Status status = source_.request_sync();
        if (status == uhal::Status::ok || status == uhal::Status::busy)
            requested_ = true;
        else
            retry_at_ = now + backoff_.next_delay_ms();
    }
}
TimeQuality TimeSyncService::quality() const
{
    return quality_;
}
std::uint32_t TimeSyncService::utc_now() const
{
    return quality_ == TimeQuality::unsynced ? 0U : system_.utc_now();
}
}  // namespace services
