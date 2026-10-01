#pragma once
#include <services/IndicationTypes.hpp>
#include <services/Limits.hpp>
#include <uhal/Status.hpp>
namespace services
{
class IIndicator
{
public:
    virtual ~IIndicator()                              = default;
    virtual uhal::Status apply(const IndicationFrame&) = 0;
};
class IndicationService final
{
public:
    explicit IndicationService(IIndicator&);
    uhal::Status request(IndicationSource, const IndicationFrame&, std::uint32_t now_ms,
                         std::uint32_t duration_ms = 0U);
    void         clear(IndicationSource);
    uhal::Status poll(std::uint32_t now_ms);

private:
    struct Request
    {
        bool            active = false;
        IndicationFrame frame{};
        std::uint32_t   expires_at = 0U;
    };
    IIndicator&     indicator_;
    Request         requests_[kIndicationSourceCount]{};
    IndicationFrame applied_{};
    bool            has_applied_ = false;
};
}  // namespace services
