#pragma once
#include <libraries/Backoff.hpp>
#include <uhal/Status.hpp>
namespace services
{
enum class TimeQuality : unsigned char
{
    unsynced,
    synced,
    stale
};
class ITimeSource
{
public:
    virtual ~ITimeSource()                        = default;
    virtual uhal::Status request_sync()           = 0;
    virtual void         poll(std::uint32_t)      = 0;
    virtual bool         has_result() const       = 0;
    virtual uhal::Status take_utc(std::uint32_t&) = 0;
};
class ISystemTime
{
public:
    virtual ~ISystemTime()                       = default;
    virtual uhal::Status  set_utc(std::uint32_t) = 0;
    virtual std::uint32_t utc_now() const        = 0;
};
class TimeSyncService final
{
public:
    TimeSyncService(ITimeSource&, ISystemTime&);
    void          on_connectivity_up(std::uint32_t);
    void          poll(std::uint32_t);
    TimeQuality   quality() const;
    std::uint32_t utc_now() const;

private:
    ITimeSource&             source_;
    ISystemTime&             system_;
    libraries::BackoffPolicy backoff_{};
    TimeQuality              quality_      = TimeQuality::unsynced;
    std::uint32_t            retry_at_     = 0U;
    std::uint32_t            last_sync_ms_ = 0U;
    bool                     requested_    = false;
};
}  // namespace services
