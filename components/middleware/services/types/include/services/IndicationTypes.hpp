#pragma once
#include <cstdint>
namespace services
{
enum class IndicationPattern : std::uint8_t
{
    off,
    solid,
    slow_blink,
    fast_blink,
    double_blink,
    breathe
};
enum class IndicationSource : std::uint8_t
{
    load_state,
    command_ack,
    connectivity,
    provisioning,
    ota,
    fault
};
struct IndicationColor
{
    std::uint8_t red   = 0U;
    std::uint8_t green = 0U;
    std::uint8_t blue  = 0U;
};
struct IndicationFrame
{
    IndicationColor   color{};
    IndicationPattern pattern   = IndicationPattern::off;
    std::uint16_t     period_ms = 0U;
};
}  // namespace services
