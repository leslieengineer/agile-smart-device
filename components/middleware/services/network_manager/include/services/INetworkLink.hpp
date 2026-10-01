#pragma once
#include <services/NetworkTypes.hpp>
#include <uhal/Status.hpp>
namespace services
{
class INetworkLink
{
public:
    virtual ~INetworkLink()                         = default;
    virtual uhal::Status start()                    = 0;
    virtual uhal::Status stop()                     = 0;
    virtual void         poll(std::uint32_t now_ms) = 0;
    virtual LinkState    state() const              = 0;
    virtual LinkQuality  quality() const            = 0;
    virtual LinkKind     kind() const               = 0;
};
class ILinkObserver
{
public:
    virtual ~ILinkObserver()                                  = default;
    virtual void on_connectivity_changed(ConnectivityState state, LinkKind link,
                                         LinkQuality quality) = 0;
};
}  // namespace services
