#pragma once
#include <cstdint>
namespace esp32c6::low_level
{
std::uint32_t system_now_ms();
void          system_sleep_ms(std::uint32_t duration_ms);
}  // namespace esp32c6::low_level
