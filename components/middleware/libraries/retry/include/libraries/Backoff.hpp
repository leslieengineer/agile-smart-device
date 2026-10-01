#pragma once

#include <cstddef>
#include <cstdint>

namespace libraries
{

class BackoffPolicy final
{
public:
    static constexpr std::size_t kDelayCount = 5U;

    BackoffPolicy() = default;
    explicit BackoffPolicy(std::uint8_t max_attempts) : max_attempts_{max_attempts} {}

    std::uint32_t next_delay_ms()
    {
        const std::size_t index = attempt_ < kDelayCount ? attempt_ : kDelayCount - 1U;
        if (attempt_ < max_attempts_)
            ++attempt_;
        return kDelaysMs[index];
    }

    void reset()
    {
        attempt_ = 0U;
    }
    bool exhausted() const
    {
        return attempt_ >= max_attempts_;
    }
    std::uint8_t attempts() const
    {
        return attempt_;
    }

private:
    inline static constexpr std::uint32_t kDelaysMs[kDelayCount] = {30000U, 60000U, 120000U,
                                                                    240000U, 300000U};
    std::uint8_t                          max_attempts_          = 10U;
    std::uint8_t                          attempt_               = 0U;
};

}  // namespace libraries
