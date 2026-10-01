#include "Esp32C6Clock.hpp"

#include "SysTimer.hpp"
namespace esp32c6::adapters
{
std::uint32_t Clock::now_ms() const
{
    return low_level::system_now_ms();
}
void Clock::sleep_ms(std::uint32_t duration_ms)
{
    low_level::system_sleep_ms(duration_ms);
}
}  // namespace esp32c6::adapters
