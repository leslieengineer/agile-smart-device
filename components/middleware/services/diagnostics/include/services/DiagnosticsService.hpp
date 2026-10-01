#pragma once
#include <cstddef>
#include <cstdint>
#include <libraries/FixedRingBuffer.hpp>
#include <services/Limits.hpp>
namespace services
{
struct FaultRecord
{
    std::uint16_t code     = 0U;
    std::uint8_t  severity = 0U;
    std::uint32_t time_ms  = 0U;
};
class IDiagnosticsSink
{
public:
    virtual ~IDiagnosticsSink()                                           = default;
    virtual void increment(std::uint8_t, std::uint32_t amount = 1U)       = 0;
    virtual void set_gauge(std::uint8_t, std::int32_t)                    = 0;
    virtual void record_fault(std::uint16_t, std::uint8_t, std::uint32_t) = 0;
};
class DiagnosticsService final : public IDiagnosticsSink
{
public:
    void          increment(std::uint8_t, std::uint32_t = 1U) override;
    void          set_gauge(std::uint8_t, std::int32_t) override;
    void          record_fault(std::uint16_t, std::uint8_t, std::uint32_t) override;
    std::uint32_t counter(std::uint8_t) const;
    std::int32_t  gauge(std::uint8_t) const;
    std::size_t   fault_count() const;

private:
    std::uint32_t counters_[kDiagnosticCounterCount]{};
    std::int32_t  gauges_[kDiagnosticGaugeCount]{};
    libraries::FixedRingBuffer<FaultRecord, kFaultRingCapacity> faults_{};
};
}  // namespace services
