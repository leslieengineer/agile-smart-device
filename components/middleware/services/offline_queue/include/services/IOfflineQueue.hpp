#pragma once

#include <services/MessageEnvelope.hpp>

namespace services
{

struct QueuedMessage
{
    char          topic[kTopicCapacity]{};
    std::uint8_t  payload[kPayloadCapacity]{};
    std::uint16_t payload_size   = 0U;
    QosLevel      qos            = QosLevel::at_most_once;
    bool          retain         = false;
    std::uint32_t correlation_id = 0U;
};

class IOfflineQueue
{
public:
    virtual ~IOfflineQueue()                                              = default;
    virtual uhal::Status enqueue(const MessageEnvelope&, bool allow_drop) = 0;
    virtual bool         peek(QueuedMessage&) const                       = 0;
    virtual void         commit_front()                                   = 0;
    virtual std::size_t  size() const                                     = 0;
};

}  // namespace services
