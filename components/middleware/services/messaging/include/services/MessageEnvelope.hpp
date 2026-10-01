#pragma once
#include <cstddef>
#include <cstdint>
#include <services/Limits.hpp>
#include <uhal/Status.hpp>
namespace services
{
enum class QosLevel : std::uint8_t
{
    at_most_once,
    at_least_once
};
struct TopicBuffer
{
    char value[kTopicCapacity]{};
};
struct PayloadView
{
    const std::uint8_t* data = nullptr;
    std::size_t         size = 0U;
};
struct MessageEnvelope
{
    TopicBuffer   topic{};
    PayloadView   payload{};
    QosLevel      qos            = QosLevel::at_most_once;
    bool          retain         = false;
    std::uint32_t correlation_id = 0U;
};
struct SessionDescriptor
{
    char          client_id[kDeviceIdCapacity]{};
    std::uint16_t keepalive_s   = 60U;
    bool          clean_session = true;
};
class IMessageHandler
{
public:
    virtual ~IMessageHandler()                      = default;
    virtual void on_message(const MessageEnvelope&) = 0;
};
class IMessageTransport
{
public:
    virtual ~IMessageTransport()                           = default;
    virtual uhal::Status connect(const SessionDescriptor&) = 0;
    virtual uhal::Status disconnect()                      = 0;
    virtual uhal::Status publish(const MessageEnvelope&)   = 0;
    virtual uhal::Status subscribe(const char*, QosLevel)  = 0;
    virtual void         poll(std::uint32_t)               = 0;
    virtual bool         is_connected() const              = 0;
    virtual void         set_handler(IMessageHandler&)     = 0;
};
}  // namespace services
