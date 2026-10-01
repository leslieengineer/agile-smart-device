#pragma once
#include <services/MessageEnvelope.hpp>
#include <uhal/Status.hpp>
namespace protocols
{
enum class MqttTopicKind : unsigned char
{
    status,
    state,
    telemetry,
    command_request,
    command_response,
    ota_request,
    ota_event,
    diagnostics
};
class MqttTopics final
{
public:
    MqttTopics(const char* site, const char* device) : site_{site}, device_{device} {}
    uhal::Status build(services::TopicBuffer&, MqttTopicKind, const char* channel = nullptr) const;

private:
    const char* site_;
    const char* device_;
};
}  // namespace protocols
