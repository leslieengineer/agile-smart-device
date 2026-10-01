#pragma once
#include <cstdint>
namespace services
{
enum class LinkKind : std::uint8_t
{
    wifi,
    cellular
};
enum class LinkState : std::uint8_t
{
    down,
    connecting,
    up,
    failed,
    disabled
};
enum class ConnectivityState : std::uint8_t
{
    offline,
    connecting,
    online_primary,
    online_backup,
    provisioning_required
};
struct LinkQuality
{
    std::int8_t  rssi_dbm   = -127;
    bool         is_metered = false;
    std::uint8_t bars       = 0U;
};
}  // namespace services
