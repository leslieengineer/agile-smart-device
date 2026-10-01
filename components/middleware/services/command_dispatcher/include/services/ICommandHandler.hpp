#pragma once
#include <cstddef>
#include <cstdint>
#include <uhal/Status.hpp>
namespace services
{
enum class CommandSource : std::uint8_t
{
    local,
    gateway,
    provisioning_portal
};
struct CommandRequest
{
    char          id[16]{};
    char          name[24]{};
    bool          value     = false;
    bool          has_value = false;
    CommandSource source    = CommandSource::gateway;
};
struct CommandResponse
{
    uhal::Status status = uhal::Status::ok;
    bool         state  = false;
};
class ICommandHandler
{
public:
    virtual ~ICommandHandler()                                           = default;
    virtual const char*  name() const                                    = 0;
    virtual uhal::Status handle(const CommandRequest&, CommandResponse&) = 0;
};
class ICommandAuthorizer
{
public:
    virtual ~ICommandAuthorizer()                               = default;
    virtual uhal::Status authorize(const CommandRequest&) const = 0;
};
}  // namespace services
