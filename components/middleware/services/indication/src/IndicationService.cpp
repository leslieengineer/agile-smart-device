#include <services/IndicationService.hpp>
namespace services
{
namespace
{
bool same(const IndicationFrame& a, const IndicationFrame& b)
{
    return a.color.red == b.color.red && a.color.green == b.color.green &&
           a.color.blue == b.color.blue && a.pattern == b.pattern && a.period_ms == b.period_ms;
}
}  // namespace
IndicationService::IndicationService(IIndicator& i) : indicator_{i} {}
uhal::Status IndicationService::request(IndicationSource source, const IndicationFrame& frame,
                                        std::uint32_t now, std::uint32_t duration)
{
    const auto index = static_cast<std::size_t>(source);
    if (index >= kIndicationSourceCount)
        return uhal::Status::invalid_argument;
    requests_[index] = {true, frame, duration == 0U ? 0U : now + duration};
    return uhal::Status::ok;
}
void IndicationService::clear(IndicationSource source)
{
    const auto index = static_cast<std::size_t>(source);
    if (index < kIndicationSourceCount)
        requests_[index].active = false;
}
uhal::Status IndicationService::poll(std::uint32_t now)
{
    for (auto& r : requests_)
        if (r.active && r.expires_at != 0U && static_cast<std::int32_t>(now - r.expires_at) >= 0)
            r.active = false;
    IndicationFrame selected{};
    bool            found = false;
    for (std::size_t i = 0U; i < kIndicationSourceCount; ++i)
        if (requests_[i].active)
        {
            selected = requests_[i].frame;
            found    = true;
        }
    if (!found)
        selected = IndicationFrame{};
    if (has_applied_ && same(selected, applied_))
        return uhal::Status::ok;
    const uhal::Status s = indicator_.apply(selected);
    if (s == uhal::Status::ok)
    {
        applied_     = selected;
        has_applied_ = true;
    }
    return s;
}
}  // namespace services
