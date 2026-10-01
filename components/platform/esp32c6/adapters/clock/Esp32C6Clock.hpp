#pragma once
#include <uhal/IClock.hpp>
namespace esp32c6::adapters
{
class Clock final : public uhal::IClock
{
public:
    std::uint32_t now_ms() const override;
    void          sleep_ms(std::uint32_t duration_ms) override;
};
}  // namespace esp32c6::adapters
