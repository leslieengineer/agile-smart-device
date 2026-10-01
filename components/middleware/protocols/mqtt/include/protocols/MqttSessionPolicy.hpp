#pragma once

#include <services/MessageEnvelope.hpp>
#include <uhal/Status.hpp>

namespace protocols
{

class MqttSessionPolicy final
{
public:
    uhal::Status build(services::SessionDescriptor& session, const char* client_id,
                       bool metered_link, bool identity_changed) const
    {
        if (client_id == nullptr || client_id[0] == '\0')
            return uhal::Status::invalid_argument;
        std::size_t index = 0U;
        while (client_id[index] != '\0')
        {
            if (index + 1U >= sizeof(session.client_id))
                return uhal::Status::no_resources;
            session.client_id[index] = client_id[index];
            ++index;
        }
        session.client_id[index] = '\0';
        session.keepalive_s      = metered_link ? 120U : 60U;
        session.clean_session    = identity_changed;
        return uhal::Status::ok;
    }
};

}  // namespace protocols
