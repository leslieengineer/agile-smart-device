#include <services/DiagnosticsService.hpp>
namespace services
{
void DiagnosticsService::increment(std::uint8_t id, std::uint32_t amount)
{
    if (id >= kDiagnosticCounterCount)
        return;
    const std::uint32_t old = counters_[id];
    counters_[id]           = 0xFFFFFFFFU - old < amount ? 0xFFFFFFFFU : old + amount;
}
void DiagnosticsService::set_gauge(std::uint8_t id, std::int32_t value)
{
    if (id < kDiagnosticGaugeCount)
        gauges_[id] = value;
}
void DiagnosticsService::record_fault(std::uint16_t code, std::uint8_t severity, std::uint32_t time)
{
    if (faults_.full())
        faults_.drop_oldest();
    faults_.try_push(FaultRecord{code, severity, time});
}
std::uint32_t DiagnosticsService::counter(std::uint8_t id) const
{
    return id < kDiagnosticCounterCount ? counters_[id] : 0U;
}
std::int32_t DiagnosticsService::gauge(std::uint8_t id) const
{
    return id < kDiagnosticGaugeCount ? gauges_[id] : 0;
}
std::size_t DiagnosticsService::fault_count() const
{
    return faults_.size();
}
}  // namespace services
