#pragma once

#include <cstddef>

namespace services
{

inline constexpr std::size_t kTopicCapacity          = 96U;
inline constexpr std::size_t kPayloadCapacity        = 256U;
inline constexpr std::size_t kDeviceIdCapacity       = 24U;
inline constexpr std::size_t kSiteIdCapacity         = 16U;
inline constexpr std::size_t kConfigKeyCapacity      = 24U;
inline constexpr std::size_t kConfigValueCapacity    = 64U;
inline constexpr std::size_t kConfigRecordCapacity   = 24U;
inline constexpr std::size_t kOfflineQueueDepth      = 32U;
inline constexpr std::size_t kSubscriptionCapacity   = 8U;
inline constexpr std::size_t kCommandHandlerCapacity = 16U;
inline constexpr std::size_t kIdempotencyWindow      = 8U;
inline constexpr std::size_t kHealthSourceCapacity   = 12U;
inline constexpr std::size_t kDiagnosticCounterCount = 32U;
inline constexpr std::size_t kDiagnosticGaugeCount   = 16U;
inline constexpr std::size_t kFaultRingCapacity      = 8U;
inline constexpr std::size_t kNetworkLinkCapacity    = 2U;
inline constexpr std::size_t kIndicationSourceCount  = 6U;
inline constexpr std::size_t kOtaChunkBytes          = 1024U;
inline constexpr std::size_t kSsidCapacity           = 33U;
inline constexpr std::size_t kPassphraseCapacity     = 65U;

}  // namespace services
