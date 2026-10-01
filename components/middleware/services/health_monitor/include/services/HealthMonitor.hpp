#pragma once
#include <cstddef>
#include <cstdint>
#include <services/Limits.hpp>
#include <uhal/IWatchdog.hpp>
namespace services
{
class IRecoveryAction
{
public:
    virtual ~IRecoveryAction()                                = default;
    virtual uhal::Status request_reboot(std::uint16_t reason) = 0;
};
class HealthMonitor final
{
public:
    HealthMonitor(uhal::IWatchdog&, IRecoveryAction&);
    uhal::Status add_source(const char* name, std::uint32_t deadline_ms, bool mandatory);
    void         check_in(std::size_t, std::uint32_t);
    uhal::Status poll(std::uint32_t now_ms);
    std::size_t  source_count() const;

private:
    struct Source
    {
        bool          used      = false;
        bool          mandatory = false;
        char          name[16]{};
        std::uint32_t deadline_ms  = 0U;
        std::uint32_t last_checkin = 0U;
    };
    uhal::IWatchdog& watchdog_;
    IRecoveryAction& recovery_;
    Source           sources_[kHealthSourceCapacity]{};
    std::size_t      count_               = 0U;
    std::uint32_t    last_reboot_request_ = 0U;
};
}  // namespace services
