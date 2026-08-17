#pragma once

#include <cstdint>

namespace board {

inline constexpr std::uint8_t kButtonPin       = 9U;
inline constexpr std::uint8_t kRelayPin        = 10U;
inline constexpr std::uint8_t kLedPin          = 2U;
inline constexpr std::uint8_t kStatusPixelPin  = 8U;
inline constexpr bool         kButtonActiveLow = true;
inline constexpr bool         kRelayActiveLow  = false;
inline constexpr bool         kLedActiveLow    = false;

}  // namespace board
