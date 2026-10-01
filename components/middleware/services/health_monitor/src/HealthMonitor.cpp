#include <services/HealthMonitor.hpp>
namespace services
{
HealthMonitor::HealthMonitor(uhal::IWatchdog& w, IRecoveryAction& r) : watchdog_{w}, recovery_{r} {}
uhal::Status HealthMonitor::add_source(const char* name, std::uint32_t deadline, bool mandatory)
{
    if (count_ >= kHealthSourceCapacity || name == nullptr)
        return uhal::Status::no_resources;
    Source& s     = sources_[count_];
    s.used        = true;
    s.mandatory   = mandatory;
    s.deadline_ms = deadline;
    std::size_t i = 0U;
    for (; i + 1U < sizeof(s.name) && name[i] != '\0'; ++i) s.name[i] = name[i];
    s.name[i] = '\0';
    ++count_;
    return uhal::Status::ok;
}
void HealthMonitor::check_in(std::size_t index, std::uint32_t now)
{
    if (index < count_)
        sources_[index].last_checkin = now;
}
uhal::Status HealthMonitor::poll(std::uint32_t now)
{
    for (std::size_t i = 0U; i < count_; ++i)
        if (sources_[i].mandatory && now - sources_[i].last_checkin > sources_[i].deadline_ms)
        {
            if (now - last_reboot_request_ >= 600000U)
            {
                last_reboot_request_ = now;
                recovery_.request_reboot(1U);
            }
            return uhal::Status::not_ready;
        }
    return watchdog_.feed();
}
std::size_t HealthMonitor::source_count() const
{
    return count_;
}
}  // namespace services
